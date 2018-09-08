#include <stdint.h>
#include <stdbool.h>
#include <hal/native_pagging.h>
#include <logging.h>
#include <core/memory.h>
#include <hal/configuration.h>
#include <core/types.h>

static inline int shifting_bits(int level)
{
    // 0 - 4096  - 12 - address shifting
    // 1 - 2M    - 21 - PTE
    // 2 - 1GB   - 30 - PDE
    // 3 - 512GB - 39 - PDPTE
    // 4 - 256T  - 48 - PML4E
    return (level) * 9 + 12; // 9 bits each level + 12 bits level 0
}

inline int index_for_level(int level, uintptr_t virtual_addr)
{
    int bits_min = shifting_bits(level-1);
    int bits_max = shifting_bits(level);
    int index = (virtual_addr % (1LL<<bits_max)) >> bits_min;
    return index;
}

/// debugging functions ///
static void log_pagetable_entry(page_map_entry_t* entry)
{
    log_debug("==> vaddr=%p \t - %p  \tpaddr=%p-%p \t size=%d KB \t %cr%c%c",
        entry->virtual_addr,
        entry->virtual_addr + entry->size - 1,
        entry->physical_addr,
        entry->physical_addr + entry->size - 1,
        entry->size/1024,
        !PERM_IS_KERNEL_MODE(entry->permission)?'u':'-',
        PERM_IS_WRITE(entry->permission)?'w':'-',
        PERM_IS_EXEC(entry->permission)?'x':'-'
    );
}

static bool accumulate_entries(int level, page_map_entry_t* acc, page_entry_t* entry, uintptr_t vaddr)
{
    uintptr_t paddr = entry->addr_12_shifted << 12;
    // merge if next segment have equal attributes
    if (acc->present &&
        acc->physical_addr+acc->size == paddr &&
        acc->virtual_addr+acc->size == vaddr &&
        PERM_IS_EXEC(acc->permission) == !entry->executeDisable &&
        PERM_IS_WRITE(acc->permission) == entry->writable &&
        PERM_IS_KERNEL_MODE(acc->permission) == !entry->user) {
        // merging
        acc->size += 1LL << shifting_bits(level-1);
        return true;
    }

    // call func only for present pages
    if (acc->present) {
        log_pagetable_entry(acc);
    }

    // refresh entry
    acc->physical_addr = paddr;
    acc->virtual_addr = vaddr;
    acc->size = 1LL << shifting_bits(level-1);
    PERM_SET_KERNEL_MODE(acc->permission, !entry->user);
    PERM_SET_WRITE(acc->permission, entry->writable);
    PERM_SET_EXEC(acc->permission, !entry->executeDisable);
    // PERM_SET_READ(acc->permission, entry->present);
    acc->present = true;
    return false;
}

static void parse_entries(int level, page_entry_t* entries, page_map_entry_t* acc, uintptr_t base_virtual_addr)
{
    // each set have PAGE_TABLE_NUMBER_OF_ENTRIES entries
    for (int index=0; index<PAGE_TABLE_NUMBER_OF_ENTRIES; index++) {
        page_entry_t entry = entries[index];
        if (entry.present) {
            uintptr_t addr = entry.addr_12_shifted << 12;
            if (level == 1 || entry.entry_or_pat) {
                accumulate_entries(level, acc, &entry, base_virtual_addr << 12);
            } else {
                parse_entries(level-1, (page_entry_t*) addr, acc, base_virtual_addr);
            }
        }

        base_virtual_addr += 1LL << ((level-1) * 9);
    }
}

static void parse_intel_memory(page_entry_t* entries)
{
    page_map_entry_t acc = {.present = 0};
    parse_entries(4, entries, &acc, 0x0);

    // is necessary flush and last accumulated entry
    if (acc.present) {
        log_pagetable_entry(&acc);
    }
}

static inline page_entry_t* get_current_page_entries()
{
    page_entry_t* mem;
    asm volatile ("movq %%cr3, %0" : "=r"(mem));
    return mem;
}

static uintptr_t allocated_aligned_memory(native_page_table_t* pt, size_t size)
{
    if (size > pt->mem_available_size) {
        // ignore the memory available. Since the allocation is in blocks, the remaining block is less than one unit
        void* new_block = kalloc(NATIVE_PAGETABLE_MEM_BUFFER_SIZE);
        linkedlist_append(pt->allocated_memory, new_block);

        pt->mem_available_size = NATIVE_PAGETABLE_MEM_BUFFER_SIZE;
        pt->mem_available_addr = ALIGN((uintptr_t) new_block, PAGE_TABLE_ENTRIES_ALIGNMENT);

        // check if memory is aligned (the ALIGN function is always reducing the memory address)
        if (pt->mem_available_addr != (uintptr_t) new_block) {
            pt->mem_available_addr += PAGE_TABLE_ENTRIES_ALIGNMENT;
            pt->mem_available_size -= pt->mem_available_addr - (uintptr_t)new_block;
        }
    }
    uintptr_t addr = pt->mem_available_addr;
    pt->mem_available_size -= size;
    pt->mem_available_addr += size;
    // log_debug("Pagetable: requested %d (0x%x) bytes. Allocated at %p. Memory available %d bytes. pt=%p", size, size, addr, pt->mem_available_size, pt);
    return addr;
}

page_entry_t* create_entries(native_page_table_t* pt)
{
    uintptr_t addr = allocated_aligned_memory(pt, sizeof(page_entry_t)*PAGE_TABLE_NUMBER_OF_ENTRIES);
    return (page_entry_t*) addr;
}

inline void fill_entry_value(page_entry_t* entry, uintptr_t ptr, bool user, bool code, bool writable)
{
    // initialize flags
    entry->present = 1;
    entry->addr_12_shifted = ptr >> 12;

    if (user) {
        entry->user = 1;
    }
    if (writable) {
        entry->writable = 1;
    }
    if (!code) {
        entry->executeDisable = 1;
    }
    return;
}

static void set_entry(native_page_table_t* pt, int level, page_entry_t* entries, uintptr_t virtual_addr, uintptr_t physical_address, bool user, bool code, bool writable)
{
    int index = index_for_level(level, virtual_addr);
    page_entry_t entry = entries[index];

    if (level == 1) {
        // log_trace("Mapped vaddr=%p paddr=%p u=%d c=%d w=%d", virtual_addr, physical_address, (int) user, (int) code, (int) writable);
        fill_entry_value(&entries[index], physical_address, user, code, writable);
    } else {
        page_entry_t* entry_ptr = (page_entry_t*) (uintptr_t) (entry.addr_12_shifted << 12);
        if (!entry.present) {
            entry_ptr = create_entries(pt);
            fill_entry_value(&entries[index], (uintptr_t) entry_ptr, user, code, writable);
        }
        set_entry(pt, level-1, entry_ptr, virtual_addr, physical_address, user, code, writable);
    }
}

void native_pagetable_dump(native_page_table_t* pt)
{
    // if pt is NULL use current ones.
    page_entry_t* entries;
    if (!pt) {
        entries = get_current_page_entries();
    } else {
        entries = pt->entries;
    }
    log_debug("=============== Dump page table begin ===============");
    parse_intel_memory(entries);
    log_debug("---------------  Dump page table end  ---------------");
}

// alocate memory to the native page structure
native_page_table_t* native_pagetable_create() {

    native_page_table_t* pt = NEW(native_page_table_t);
    pt->allocated_memory = linkedlist_create();
    pt->mem_available_addr = 0;
    pt->mem_available_size = 0;
    pt->entries = create_entries(pt);
    return pt;
}

/**
 * Add a new memory mapping to the native page structure.
 */
void native_pagetable_set(native_page_table_t* pt, page_map_entry_t entry)
{
    if (entry.virtual_addr != MEM_ALIGN(entry.virtual_addr) || entry.physical_addr != MEM_ALIGN(entry.physical_addr)) {
        log_warn("Invalid paging definition. Memory address is not aligned: %p -> %p", entry.virtual_addr, entry.physical_addr);
        return;
    }

    page_entry_t* entries_l4 = pt->entries;
    // TODO Add Support to big pages
    for (size_t i=0; i<entry.size / PAGE_TABLE_NATIVE_SIZE_SMALL; i++) {
        set_entry(pt, 4, entries_l4, entry.virtual_addr, entry.physical_addr, !PERM_IS_KERNEL_MODE(entry.permission), 
            PERM_IS_EXEC(entry.permission), PERM_IS_WRITE(entry.permission));
    }
}

/**
 * Switch the current processor to the page tables.
 */
void native_pagetable_switch(native_page_table_t* pt)
{
    if ((uintptr_t) get_current_page_entries() != (uintptr_t) pt->entries) {
        native_pagetable_dump(pt);
        asm volatile ("movq %0, %%cr3" : : "r" (pt->entries));
    }
}

void native_page_table_flush()
{
    // TODO Implement a more efficient version: https://www.felixcloutier.com/x86/INVLPG.html
    asm volatile ("movq	%%cr3, %%rax; movq %%rax, %%cr3"
        :
        :
        : "rax");
    return;
}

