#ifndef _CORE_HAL_NATIVE_VMEM_H_
#define _CORE_HAL_NATIVE_VMEM_H_
#include <stdint.h>
#include <core/types.h>
#include <core/vmem/services.h>

typedef struct {
    uintptr_t virtual_addr;
    uintptr_t physical_addr;
    size_t size;
    permission_t permission;
    bool present;
} page_map_entry_t;

/**
 * Creates a native structure to implements the virtual memory.
 */
bool native_vmem_create(vmem_t* vmem);

/**
 * Physically maps the virtual memory to a physical address.
 */
bool native_vmem_set(vmem_t* vmem, page_map_entry_t entry);

/**
 * Switch to the new virtual memory mapping.
 */
void native_vmem_switch(vmem_t* vmem);

/**
 * Flush the virtual memory mapping currently in use.
 */
void native_vmem_flush();

/**
 * Dumps the native paging mapping into the logging.
 * param @vmem: virtual memory to dump pages for. If this value is null, the current page tables are dumped.
 */
void native_vmem_dump(vmem_t* vmem);


#endif