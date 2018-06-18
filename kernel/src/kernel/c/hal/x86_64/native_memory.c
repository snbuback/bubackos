#include <stdint.h>
#include <stdbool.h>
#include <hal/hal.h>
#include <core/logging.h>
#include <core/memory.h>
#include <core/configuration.h>

page_entry_t* create_entries()
{
    // TODO creates memory allocation aligned
    uintptr_t addr = (uintptr_t) kmem_alloc(sizeof(page_entry_t)*PAGE_TABLE_NUMBER_OF_ENTRIES + PAGE_TABLE_ALIGN);
    addr += PAGE_TABLE_ALIGN;
    addr = ALIGN(addr, PAGE_TABLE_ALIGN);
    log_debug("Page entries at %p", addr);
    return (page_entry_t*) addr;
}

// alocate memory to the native page structure
native_page_table_t* hal_page_table_create_mapping() {

    native_page_table_t* pt = (native_page_table_t*) kmem_alloc(sizeof(native_page_table_t));
    pt->entries = create_entries();
    pt->cr3 = (uintptr_t) pt->entries;
    return pt;
}

int index_for_level(int level, uintptr_t virtual_addr)
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

void fill_entry_value(page_entry_t* entry, uintptr_t ptr, bool user, bool code, bool writable)
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
    // log_trace("Level %d entries=%p virtual_addr=%p physical_addr=%p user=%d code=%d : (index=%d)", level, entries, virtual_addr, physical_address, (int) user, (int) code, index);
    page_entry_t entry = entries[index];

    if (level == 1) {
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
    // DEBUGGER();
    asm volatile ("movq %0, %%cr3" : : "r" (hal_mmap->cr3));
}

// utility functions (used to debug)
static void print_entry(page_entry_t entry, uintptr_t virtual_addr)
{
    log_debug("Entry: virtual=%p addr=%p present=%d writable=%d user=%d executeDisable=%d\n", 
        virtual_addr, (entry.addr_12_shifted << 12), (int) entry.present, (int) entry.writable, (int) entry.user, (int) entry.executeDisable);
}

void parse_entries(int level, page_entry_t* entries, uintptr_t base_virtual_addr, entry_visited_func func)
{
    // each set have PAGE_TABLE_NUMBER_OF_ENTRIES entries
    for (int index=0; index<PAGE_TABLE_NUMBER_OF_ENTRIES; index++) {
        page_entry_t entry = entries[index];
        if (entry.present) {
            uintptr_t addr = entry.addr_12_shifted << 12;
            if (level == 1 || entry.entry_or_pat) {
                func(entry, base_virtual_addr << 12);
            } else {
                parse_entries(level-1, (page_entry_t*) addr, base_virtual_addr, func);
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
    parse_entries(4, entries, 0x0, func);
}

