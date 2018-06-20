#ifndef __HAL_NATIVE_MEMORY_H
#define __HAL_NATIVE_MEMORY_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// x86_64 memory mapping is organized as array of arrays
#define PAGE_TABLE_NUMBER_OF_ENTRIES    1024
#define PAGE_TABLE_ALIGN                4096

#define ALIGN(addr, a)                  (addr & ~(a - 1))
#define PAGE_ENTRY_ADDR_MASK            0x0000fffffffff000L

#define PAGE_ENTRY_PRESENT				(1LL << 0)
#define PAGE_ENTRY_WRITABLE				(1LL << 1)
#define PAGE_ENTRY_USER					(1LL << 2)
#define PAGE_ENTRY_WRITE_THROUGH		(1LL << 3)
#define PAGE_ENTRY_CACHING_DISABLED		(1LL << 4)
#define PAGE_ENTRY_ACCESSED				(1LL << 5)
#define PAGE_ENTRY_DIRTY				(1LL << 6)
#define PAGE_ENTRY_NOT_EXECUTABLE		(1LL << 63)

typedef struct {
    bool present: 1;
    bool writable: 1;
    bool user: 1;
    bool write_through: 1;
    bool cache_disable: 1;
    bool accessed: 1;
    bool dirty: 1;
    bool entry_or_pat: 1;
    int : 4; // ignored
    uint64_t addr_12_shifted: 48; // 12 bits shifted right address
    int : 3;
    bool executeDisable: 1;    
} page_entry_t;

typedef struct {
    uint64_t cr3;
    void* entries;
} native_page_table_t;

typedef struct {
    uintptr_t vaddr;
    uintptr_t paddr;
    size_t size;
    bool user;
    bool code;
    bool writable;
    bool present;
} page_map_entry_t;

typedef void (*entry_visited_func)(page_map_entry_t* entry);

void initialize_native_page_table(uintptr_t kernel_start, size_t kernel_size);

// alocate memory to the native page structure
native_page_table_t* hal_page_table_create_mapping();

/**
 * Add a new memory mapping to the native page structure.
 * This function is called with one 1 page.
 */
void hal_page_table_add_mapping(native_page_table_t* hal_mmap, uintptr_t virtual_address, uintptr_t physical_address, bool user, bool code, bool writable);

/**
 * Remove a memory mapping.
 * This function is called with one 1 page.
 */
void hal_page_table_del_mapping(native_page_table_t* hal_mmap, uintptr_t virtual_address);

/**
 * Release the memory map entries
 */
void hal_page_table_destroy_mapping(native_page_table_t* hal_mmap);

/**
 * Makes the mmap active
 */
void hal_switch_mmap(native_page_table_t* hal_mmap);


/**
 * Utility function for debugging propose
 */
void parse_intel_memory(page_entry_t* entries, entry_visited_func func);

// TODO internal function, but used by test.
int index_for_level(int level, uintptr_t virtual_addr);
void fill_entry_value(page_entry_t* entry, uintptr_t ptr, bool user, bool code, bool writable);
page_entry_t* create_entries();



#endif