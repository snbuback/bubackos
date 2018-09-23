#ifndef _CORE_VMEM_REGION_SERVICES_H_
#define _CORE_VMEM_REGION_SERVICES_H_
#include <algorithms/linkedlist.h>
#include <libutils/id_mapper.h>
#include <core/vmem/services.h>

typedef id_handler_t vmem_region_id_t;

typedef struct {
    vmem_region_id_t vmem_region_id;
    linkedlist_t* attached; // list of memory_t*
    const char* name;
    uintptr_t start;
    size_t size;
    size_t allocated_size;
    bool user;
    bool writable;
    bool executable;
    linkedlist_t* pages; // uintptr_t (address of the physical memory)
} vmem_region_t;

bool vmem_region_initialize();

/** 
 * Creates a new region of memory.
 * @params start: virtual start address (memory aligned) or 0 for no preference.
 */
vmem_region_t* vmem_region_create(vmem_t* vmem, const char* name, uintptr_t start, size_t size, bool user, bool writable, bool executable);

/**
 * Resizes a memory region, releasing/alocating pages as needed.
 */
bool vmem_region_resize(vmem_region_t* vmem_region, size_t new_size);

/**
 * Maps a range of physical address to the memory region.
 */
uintptr_t vmem_region_map_physical_address(vmem_region_t* vmem_region, uintptr_t physical_start_addr, size_t size);

/**
 * Returns the current size of the vmemory region.
 */
static inline size_t vmem_region_current_size(vmem_region_t* vmem_region)
{
    return vmem_region ? vmem_region->size : 0;
}

/**
 * Display the mapping of the region in the console.
 */
void vmem_region_dump(vmem_region_t* vmem_region);

#endif