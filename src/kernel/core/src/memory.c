#include <string.h>
#include <logging.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hal/platform.h>
#include <core/memory_management.h>
#include <core/page_allocator.h>

static memory_region_t* data_mem_region = NULL;
void* current_mem_ptr = NULL;

bool memory_allocator_initialize()
{
    memory_t* kernel_mem = memory_management_get_kernel();
    // associates all memory consumed so far to the kernel memory region

    // TODO Since hal_switch_task uses pushq it is required execution permission on kernel memory
    uintptr_t data_addr = platform.memory.kernel_data.addr_start;
    memory_region_t* new_data_mem_region = memory_management_region_create(
        kernel_mem, 
        data_addr,
        0,
        false,
        true,
        true);  // hal_switch_task
    if (!new_data_mem_region) {
        log_fatal("Error requesting memory for kernel data");
        return false;
    }

    size_t associate_memory = 0;
    while (associate_memory < platform.memory.kernel_data.size) {
        if (!page_allocator_mark_as_system(data_addr, SYSTEM_PAGE_SIZE)) {
            return false;
        }
        if (!memory_management_map_physical_address(new_data_mem_region, data_addr)) {
            return false;
        }
        associate_memory += SYSTEM_PAGE_SIZE;
        data_addr += SYSTEM_PAGE_SIZE;
    }

    // since the condition to check if memory module was initialize is data_mem_region, it should be the last
    // attribution, marking the memory module was initialised.
    data_mem_region = new_data_mem_region;
    return true;
}

static bool increase_kernel_data(size_t size)
{
    if (data_mem_region) {
        // increments page by page instead of by byte
        size_t new_size = ALIGN_NEXT(size, SYSTEM_PAGE_SIZE);
        log_info("Resizing kernel data memory by %d (requested %d)", new_size, size);
        if (!memory_management_region_resize(data_mem_region, platform.memory.kernel_data.size)) {
            return false;
        }

        platform.memory.kernel_data.addr_end += new_size;
        platform.memory.kernel_data.size += new_size;
    } else {
        // since the memory allocator was not initialize yet, the allocation is byte by byte
        platform.memory.kernel_data.addr_end += size;
        platform.memory.kernel_data.size += size;
    }
    return true;
}

static inline void* get_current_memory_ptr()
{
    if (!current_mem_ptr) {
        current_mem_ptr = (void*) platform.memory.kernel_data.addr_end;
    }
    return current_mem_ptr;
}

/**
 * Kernel memory allocator. All memory allocated is filled by 0 by default.
 */
void* malloc(size_t size)
{
    void *ptr = get_current_memory_ptr();
    if (ptr + size > (void*) platform.memory.kernel_data.addr_end) {
        if (!increase_kernel_data(size)) {
            log_fatal("-*-*-*-*-*-*-*-*-* OutOfMemory-*-*-*-*-*-*-*-*-*-*-*-*");
            return NULL;
        }
    }

    // increase memory pointer
    current_mem_ptr += size;

    if (size >= 64) {
        log_trace("*** Allocated %d bytes (0x%x) at %p (%d KB). Used %d KB", size, size, ptr, (uintptr_t) ptr/1024, platform.memory.kernel_data.size/1024);
    }
    memset(ptr, 0, size);
    return ptr;
}

/**
 * Release
 */
void free(void *addr)
{
    if (addr != NULL) {
        // release
    }
    // log_info("Releasing memory at %p (dummy). Used %d MB", addr, total_allocated/1024/1024);
    // TODO not implemented yet
}
