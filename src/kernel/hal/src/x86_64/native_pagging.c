#include <stdint.h>
#include <stdbool.h>
#include <hal/native_pagging.h>
#include <core/logging.h>
#include <core/memory.h>
#include <core/configuration.h>
#include <core/types.h>

// TODO this is a nightware. Refactor this code. Maybe a debug routine?
// utility functions (used to debug) -- this function should be part of the core since it is agnostic to the memory page structure
static void print_entry(page_map_entry_t* entry)
{
    log_debug("==> vaddr=%p-%p paddr=%p size=%d KB %cr%c%c", 
        entry->virtual_addr, entry->virtual_addr + entry->size, entry->physical_addr, entry->size/1024, !PERM_IS_KERNEL_MODE(entry->permission)?'u':'-', 
        PERM_IS_WRITE(entry->permission)?'w':'-', PERM_IS_EXEC(entry->permission)?'x':'-');
}

bool accumulate_entries(page_map_entry_t* acc, page_entry_t* entry, uintptr_t vaddr, entry_visited_func func)
{
    uintptr_t paddr = entry->addr_12_shifted << 12;
    // merge if memory is contigous
    if (acc->present && acc->physical_addr+acc->size == paddr && PERM_IS_EXEC(acc->permission) == !entry->executeDisable 
        && PERM_IS_WRITE(acc->permission) == entry->writable &&  PERM_IS_KERNEL_MODE(acc->permission) == !entry->user) {
        // merging
        acc->size += PAGE_TABLE_NATIVE_SIZE_SMALL;
        return true;
    }

    // call func only for 
    if (acc->present) {
        func(acc);
    }

    // update the entry
    acc->physical_addr = paddr;
    acc->virtual_addr = vaddr;
    acc->size = SYSTEM_PAGE_SIZE;
    PERM_SET_KERNEL_MODE(acc->permission, !entry->user);
    PERM_SET_WRITE(acc->permission, entry->writable);
    PERM_SET_EXEC(acc->permission, !entry->executeDisable);
    // PERM_SET_READ(acc->permission, entry->present);
    acc->present = true;
    return false;
}

void parse_entries(int level, page_entry_t* entries, page_map_entry_t* acc, uintptr_t base_virtual_addr, entry_visited_func func)
{
    // each set have PAGE_TABLE_NUMBER_OF_ENTRIES entries
    for (int index=0; index<PAGE_TABLE_NUMBER_OF_ENTRIES; index++) {
        page_entry_t entry = entries[index];
        if (entry.present) {
            uintptr_t addr = entry.addr_12_shifted << 12;
            if (level == 1 || entry.entry_or_pat) {
                accumulate_entries(acc, &entry, base_virtual_addr << 12, func);
            } else {
                parse_entries(level-1, (page_entry_t*) addr, acc, base_virtual_addr, func);
            }
        }

        base_virtual_addr += 1LL << ((level-1) * 9);
    }
}

void parse_intel_memory(page_entry_t* entries, entry_visited_func func)
{
    if (func == NULL) {
        func = print_entry;
    }
    log_debug("========== Dump page table begin ==========");
    page_map_entry_t acc = {.present = 0};
    parse_entries(4, entries, &acc, 0x0, func);

    // is necessary flush and last accumulated entry
    if (acc.present) {
        func(&acc);
    }
    log_debug("--------- Dump page table end ---------");
}

static inline page_entry_t* get_current_page_entries()
{
    page_entry_t* mem;
    asm volatile ("movq %%cr3, %0" : "=r"(mem));
    return mem;
}

void dump_current_page_table() {
    parse_intel_memory(get_current_page_entries(), NULL);
}

page_entry_t* create_entries()
{
    // TODO !important!!! creates memory allocation aligned
    uintptr_t addr = (uintptr_t) kmem_alloc(sizeof(page_entry_t)*PAGE_TABLE_NUMBER_OF_ENTRIES + PAGE_TABLE_ALIGN);
    addr += PAGE_TABLE_ALIGN;
    addr = ALIGN(addr, PAGE_TABLE_ALIGN);
    // log_debug("Page entries at %p", addr);
    return (page_entry_t*) addr;
}

// alocate memory to the native page structure
native_page_table_t* native_pagetable_create() {

    native_page_table_t* pt = (native_page_table_t*) kmem_alloc(sizeof(native_page_table_t));
    pt->entries = create_entries();
    return pt;
}

inline int index_for_level(int level, uintptr_t virtual_addr)
{
    // 0 - 4096 - 12
    // 1 - 2M  - 21
    // 2 - 1GB - 30
    // 3 - 512GB - 39
    // 4 - 256T - 48

    int bits_min = (level-1) * 9 + 12; // 9 bits each level + 12 bits level 0
    int bits_max = (level) * 9 + 12; // 9 bits each level + 12 bits level 0
    int index = (virtual_addr % (1LL<<bits_max)) >> bits_min;
    return index;
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

static void set_entry(int level, page_entry_t* entries, uintptr_t virtual_addr, uintptr_t physical_address, bool user, bool code, bool writable)
{
    int index = index_for_level(level, virtual_addr);
    page_entry_t entry = entries[index];

    if (level == 1) {
        // log_trace("Mapped vaddr=%p paddr=%p u=%d c=%d w=%d", virtual_addr, physical_address, (int) user, (int) code, (int) writable);
        fill_entry_value(&entries[index], physical_address, user, code, writable);
    } else {
        page_entry_t* entry_ptr = (page_entry_t*) (uintptr_t) (entry.addr_12_shifted << 12);
        if (!entry.present) {
            entry_ptr = create_entries();
            fill_entry_value(&entries[index], (uintptr_t) entry_ptr, user, code, writable);
        }
        set_entry(level-1, entry_ptr, virtual_addr, physical_address, user, code, writable);
    }
}

/**
 * Add a new memory mapping to the native page structure.
 */
void native_pagetable_set(native_page_table_t* pt, page_map_entry_t entry)
{
    page_entry_t* entries_l4 = pt->entries;
    // TODO Add Support to big pages
    for (size_t i=0; i<entry.size / PAGE_TABLE_NATIVE_SIZE_SMALL; i++) {
        set_entry(4, entries_l4, entry.virtual_addr, entry.physical_addr, !PERM_IS_KERNEL_MODE(entry.permission), 
            PERM_IS_EXEC(entry.permission), PERM_IS_WRITE(entry.permission));
    }
}

/**
 * Switch the current processor to the page tables.
 */
void native_pagetable_switch(native_page_table_t* pt)
{
    if ((uintptr_t) get_current_page_entries() != (uintptr_t) pt->entries) {
        asm volatile ("movq %0, %%cr3" : : "r" (pt->entries));
    }
}

