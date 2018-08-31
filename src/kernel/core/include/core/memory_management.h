#ifndef _MEMORY_MANAGEMENT_H_
#define _MEMORY_MANAGEMENT_H_
#include <stdint.h>
#include <stdlib.h>
#include <algorithms/linkedlist.h>
#include <hal/native_pagging.h>
#include <libutils/utils.h>
#include <hal/configuration.h>

typedef unsigned int memory_id_t;
typedef unsigned int region_id_t;

typedef struct {
    memory_id_t id;
    native_page_table_t* pt;
    linkedlist_t* regions; // list of memory_region_t
    linkedlist_t* map; // list of memory_map_t
    uintptr_t next_start_address;
} memory_t;

typedef struct {
    memory_t* memory;
    const char* region_name;
    uintptr_t start;
    size_t size;
    size_t allocated_size;
    bool user;
    bool writable;
    bool executable;
    linkedlist_t* pages; // uintptr_t (address of the physical memory)
} memory_region_t;

typedef struct {
    uintptr_t virtual_addr;
    uintptr_t physical_addr;
    memory_region_t* region;
} memory_map_t;

extern memory_region_t* kernel_code_region;
extern memory_region_t* kernel_data_region;

/**
 * Initialize the memory management system.
 * Also creates the first map, mapping the kernel memory
 */
bool memory_management_initialize();

/**
 * Creates a new memory management area
 */
memory_t* memory_management_create();

/** 
 * Creates a new region of memory.
 * @params start: virtual start address (memory aligned) or 0 for no preference.
 */
memory_region_t* memory_management_region_create(memory_t* memory, const char* region_name, uintptr_t start, size_t size, bool user, bool writable, bool executable);

/**
 * Resizes a memory region, releasing/alocating pages as needed.
 */
bool memory_management_region_resize(memory_region_t* region, size_t new_size);

uintptr_t memory_management_region_map_physical_address(memory_region_t* region, uintptr_t physical_start_addr, size_t size);

size_t memory_management_region_current_size(memory_region_t* region);

bool memory_management_region_destroy(memory_region_t* region);

void memory_management_destroy(memory_t* memory);

uintptr_t memory_management_get_physical_address(memory_t* mhandler, uintptr_t vaddr);

memory_t* memory_management_get_kernel();

void memory_management_dump(memory_t* memory);

void memory_management_region_dump(memory_region_t* region);

#endif