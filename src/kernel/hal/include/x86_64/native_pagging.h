#ifndef __HAL_NATIVE_MEMORY_H
#define __HAL_NATIVE_MEMORY_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libutils/utils.h>
#include <algorithms/linkedlist.h>

// x86_64 memory mapping is organized as array of arrays
#define PAGE_TABLE_NUMBER_OF_ENTRIES    512   // 64 bits 512 entry - 32 bits 1024 entries
#define PAGE_TABLE_ENTRIES_ALIGNMENT    4096

#define PAGE_TABLE_NATIVE_SIZE_SMALL          4096      // 4K
#define PAGE_TABLE_NATIVE_SIZE_BIG            2097152   // 2M

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

#define NATIVE_PAGETABLE_MEM_BUFFER_SIZE    (10*PAGE_TABLE_NUMBER_OF_ENTRIES*sizeof(page_entry_t))

typedef struct {
    void* entries;

    // since my allocation system doesn't support allocate memory ensure aligned, the trick is allocate more
    // than required and do a manual allocation. To avoid waste memory, I'm allocating buffer of 
    // 10 times the size of page entries
    linkedlist_t* allocated_memory;
    uintptr_t mem_available_addr;
    size_t mem_available_size;
} native_page_table_t;

int index_for_level(int level, uintptr_t virtual_addr);
void fill_entry_value(page_entry_t* entry, uintptr_t ptr, bool user, bool code, bool writable);
page_entry_t* create_entries();


#endif