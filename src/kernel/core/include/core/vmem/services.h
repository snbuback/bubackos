#ifndef _CORE_VMEM_SERVICES_H_
#define _CORE_VMEM_SERVICES_H_
#include <stdalign.h>
#include <algorithms/linkedlist.h>
#include <libutils/id_mapper.h>
#include <libutils/utils.h>
#include <hal/configuration.h>

typedef id_handler_t vmem_id_t;

#define REGION_ADDRESS_INCREMENT        (1*1024*1024)   // 1GB
#define MEM_ALIGN(addr)			        ALIGN(addr, SYSTEM_PAGE_SIZE)

typedef struct {
    vmem_id_t vmem_id;
    void* native_vmem;
    linkedlist_t* regions; // list of memory_region_t
    // TODO move the map of memory to regions
    linkedlist_t* map; // list of memory_map_t
    uintptr_t next_start_address;
} vmem_t;

#include <core/vmem/region_services.h>

// TODO Only dependency between region_services and the services.c is the vmem_region_t
// After move this to inside each vmem_region, probably this dependency will disappear.
typedef struct {
    uintptr_t virtual_addr;
    uintptr_t physical_addr;
    vmem_region_t* region;
} vmem_map_t;

/**
 * Initialize the virtual memory system.
 * Also creates the first map, mapping the kernel memory
 */
bool vmem_initialize();

/**
 * Creates a virtual memory area.
 */
vmem_t* vmem_create();

/**
 * Returns the vmem_t* object associate to a given id.
 */
vmem_t* vmem_get(vmem_id_t vmem_id);

/**
 * Attach a region of memory to a virtual memory area.
 */
bool vmem_attach(vmem_t* vmem, vmem_region_t* vmem_region);

/**
 * Releases the virtual memory area.
 */
void vmem_destroy(vmem_t* vmem);

/**
 * Returns the physical address of a memory virtual memory area.
 */
uintptr_t vmem_get_physical_address(vmem_t* vmem, uintptr_t vaddr);

bool vmem_notify_change(vmem_t* vmem, vmem_region_t* vmem_region, uintptr_t paddr, uintptr_t vaddr);

/**
 * Returns the virtual memory area of the kernel.
 */
vmem_t* vmem_get_kernel();

/**
 * Dumps the regions of a virtual memory area.
 */
void vmem_dump(vmem_t* memory);

#endif