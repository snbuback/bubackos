#ifndef _MEMORY_MANAGEMENT_H_
#define _MEMORY_MANAGEMENT_H_
#include <stdint.h>
#include <stdlib.h>
#include <algorithms/linkedlist.h>
#include <hal/hal.h>

#define MEMORY_MANAGEMENT_MAX_NUMBER_OF_REGIONS     10
#define MEM_ALIGN(addr)			ALIGN(addr, SYSTEM_PAGE_SIZE)

typedef unsigned int memory_id_t;
typedef unsigned int region_id_t;

typedef struct {
    memory_id_t id;
    native_page_table_t* pt;
    linkedlist_t* regions; // list of memory_region_t
    linkedlist_t* map; // list of memory_map_t
} memory_t;

typedef struct {
    memory_t* memory;
    uintptr_t start;
    size_t size;
    bool user;
    bool writable;
    bool executable;
    linkedlist_t* pages;
} memory_region_t;

typedef struct {
    uintptr_t virtual_addr;
    uintptr_t physical_addr;
    memory_region_t* region;
} memory_map_t;

/**
 * Initialize the memory management system.
 * Also creates the first map, mapping the kernel memory
 */
void memory_management_initialize();

/**
 * Creates a new memory management area
 */
memory_t* memory_management_create();

/** 
 * Creates a new region of memory.
 * @params start: virtual start address (memory aligned) or 0 for no preference.
 */
memory_region_t* memory_management_region_create(memory_t* memory, uintptr_t start, size_t size, bool user, bool writable, bool executable);

bool memory_management_region_resize(memory_region_t* region, size_t new_size);

bool memory_management_region_current_size(memory_region_t* region, region_id_t r_id);

bool memory_management_region_destroy(memory_region_t* region);

void memory_management_destroy(memory_t* memory);

uintptr_t memory_management_get_physical_address(memory_t* mhandler, uintptr_t vaddr);

#endif