#include <stdint.h>
#include <stdbool.h>
#include <hal/native_pagging.h>
#include <core/logging.h>
#include <core/memory.h>
#include <core/configuration.h>
#include <core/types.h>

static inline int shifting_bits(int level)
{
    // 0 - 4096 - 12
    // 1 - 2M  - 21
    // 2 - 1GB - 30
    // 3 - 512GB - 39
    // 4 - 256T - 48
    return (level) * 9 + 12; // 9 bits each level + 12 bits level 0
}

static inline int index_for_level(int level, uintptr_t virtual_addr)
{

    int bits_min = shifting_bits(level-1);
    int bits_max = shifting_bits(level);
    int index = (virtual_addr % (1LL<<bits_max)) >> bits_min;
    return index;
}

/// debugging functions ///
static void log_pagetable_entry(page_map_entry_t* entry)
{
    log_debug("==> vaddr=%p-%p paddr=%p size=%d KB %cr%c%c", 
        entry->virtual_addr, entry->virtual_addr + entry->size, entry->physical_addr, entry->size/1024, !PERM_IS_KERNEL_MODE(entry->permission)?'u':'-', 
        PERM_IS_WRITE(entry->permission)?'w':'-', PERM_IS_EXEC(entry->permission)?'x':'-');
}

static bool accumulate_entries(int level, page_map_entry_t* acc, page_entry_t* entry, uintptr_t vaddr)
{
    uintptr_t paddr = entry->addr_12_shifted << 12;
    // merge if next segment have equal attributes
    if (acc->present && acc->physical_addr+acc->size == paddr && PERM_IS_EXEC(acc->permission) == !entry->executeDisable 
        && PERM_IS_WRITE(acc->permission) == entry->writable &&  PERM_IS_KERNEL_MODE(acc->permission) == !entry->user) {
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

static page_entry_t* create_entries()
{
    // TODO !important!!! creates memory allocation aligned
    uintptr_t addr = (uintptr_t) kmem_alloc(sizeof(page_entry_t)*PAGE_TABLE_NUMBER_OF_ENTRIES + PAGE_TABLE_NATIVE_SIZE_SMALL);
    addr += PAGE_TABLE_NATIVE_SIZE_SMALL;
    addr = ALIGN(addr, PAGE_TABLE_NATIVE_SIZE_SMALL);
    // log_debug("Page entries at %p", addr);
    return (page_entry_t*) addr;
}

static inline void fill_entry_value(page_entry_t* entry, uintptr_t ptr, bool user, bool code, bool writable)
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

void native_pagetable_dump(native_page_table_t* pt)
{
    // if pt is NULL use current ones.
    page_entry_t* entries;
    if (!pt) {
        entries = get_current_page_entries();
    } else {
        entries = pt->entries;
    }
    log_debug("========== Dump page table begin ==========");
    parse_intel_memory(entries);
    log_debug("--------- Dump page table end ---------");
}

// alocate memory to the native page structure
native_page_table_t* native_pagetable_create() {

    native_page_table_t* pt = (native_page_table_t*) kmem_alloc(sizeof(native_page_table_t));
    pt->entries = create_entries();
    return pt;
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

