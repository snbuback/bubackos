#include <stdint.h>
#include <stdbool.h>
#include <hal/hal.h>
#include <core/logging.h>
#include <core/memory.h>
#include <core/configuration.h>

// TODO this is a nightware. Refactor this code. Maybe a debug routine?
// utility functions (used to debug) -- this function should be part of the core since it is agnostic to the memory page structure
static void print_entry(page_map_entry_t* entry)
{
    log_debug("==> vaddr=%p paddr=%p size=%p (%d KB) %cr%c%c", 
        entry->vaddr, entry->paddr, entry->size, entry->size/1024, entry->user?'u':'-', entry->writable?'w':'-', entry->code?'x':'-');
}

bool accumulate_entries(page_map_entry_t* acc, page_entry_t* entry, uintptr_t vaddr, entry_visited_func func)
{
    uintptr_t paddr = entry->addr_12_shifted << 12;
    // merge if memory is contigous
    if (acc->present && acc->paddr+acc->size == paddr && acc->code == !entry->executeDisable 
        && acc->writable == entry->writable && acc->user == entry->user) {
        // merging
        acc->size += SYSTEM_PAGE_SIZE;
        return true;
    }

    // call func only for 
    if (acc->present) {
        func(acc);
    }

    // update the entry
    acc->paddr = paddr;
    acc->vaddr = vaddr;
    acc->size = SYSTEM_PAGE_SIZE;
    acc->user = entry->user;
    acc->writable = entry->writable;
    acc->code = !entry->executeDisable;
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
native_page_table_t* hal_page_table_create_mapping() {

    native_page_table_t* pt = (native_page_table_t*) kmem_alloc(sizeof(native_page_table_t));
    pt->entries = create_entries();
    pt->cr3 = (uintptr_t) pt->entries;
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
 * This function is called with one 1 page.
 */
void hal_page_table_add_mapping(native_page_table_t* hal_mmap, uintptr_t virtual_address, uintptr_t physical_address, bool user, bool code, bool writable)
{
    page_entry_t* entries_l4 = hal_mmap->entries;
    set_entry(4, entries_l4, virtual_address, physical_address, user, code, writable);
}

/**
 * Remove a memory mapping.
 * This function is called with one 1 page.
 */
// void hal_page_table_del_mapping(native_page_table_t hal_mmap, uintptr_t virtual_address);

/**
 * Release the memory map entries
 */
// void hal_page_table_destroy_mapping(native_page_table_t hal_mmap);

/**
 * Makes the mmap active
 */
void hal_switch_mmap(native_page_table_t* hal_mmap)
{
    if ((uintptr_t) get_current_page_entries() != hal_mmap->cr3) {
        asm volatile ("movq %0, %%cr3" : : "r" (hal_mmap->cr3));
    }
}

