#include <string.h>
#include <logging.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hal/platform.h>
#include <core/memory_management.h>
#include <core/page_allocator.h>

#define KERNEL_MEMORY_PAGE_RESERVE       5

memory_region_t* data_mem_region = NULL;
static void* current_mem_ptr = NULL;

bool memory_allocator_initialize()
{
    memory_t* kernel_mem = memory_management_get_kernel();
    // associates all memory consumed so far to the kernel memory region

    // TODO Since hal_switch_task uses pushq it is required execution permission on kernel memory
    uintptr_t data_addr = platform.memory.kernel_data.addr_start;
    memory_region_t* new_data_mem_region = memory_management_region_create(
        kernel_mem,
        "kernel-data",
        data_addr,
        0,
        false,
        true,
        true);  // hal_switch_task
    if (!new_data_mem_region) {
        log_fatal("Error requesting memory for kernel data");
        return false;
    }

    size_t num_pages_consumed = (platform.memory.kernel_data.size / SYSTEM_PAGE_SIZE) + 1 + KERNEL_MEMORY_PAGE_RESERVE;
    uintptr_t paddrs[num_pages_consumed];
    for (size_t i=0; i<num_pages_consumed; ++i) {
        if (!page_allocator_mark_as_system(data_addr, SYSTEM_PAGE_SIZE)) {
            return false;
        }
        paddrs[i] = data_addr;
        data_addr += SYSTEM_PAGE_SIZE;
    }
    if (!memory_management_map_physical_address(new_data_mem_region, num_pages_consumed, paddrs)) {
        return false;
    }

    // if during the memory allocation the kernel data memory grows more than allocated, there is an error
    size_t allocated_size = memory_management_region_current_size(new_data_mem_region);
    if (platform.memory.kernel_data.size > allocated_size) {
        log_warn("Memory grows more than the allocated: total %d > allocated %d", platform.memory.kernel_data.size, allocated_size);
        return false;
    }

    // since the condition to check if memory module was initialize is data_mem_region, it should be the last
    // attribution, marking the memory module was initialised.
    data_mem_region = new_data_mem_region;
    return true;
}

static inline void* get_current_memory_ptr()
{
    if (!current_mem_ptr) {
        current_mem_ptr = (void*) platform.memory.kernel_data.addr_end;
    }
    return current_mem_ptr;
}

static bool ensure_free_memory(size_t size)
{
    size_t minimum_required_size = (uintptr_t) get_current_memory_ptr() + size - platform.memory.kernel_data.addr_start;
    size_t current_size = platform.memory.kernel_data.size;

    if (minimum_required_size > current_size) {
        // increments memory taking into account the reserved memory
        size_t new_size = ALIGN_NEXT(current_size + size, SYSTEM_PAGE_SIZE) + KERNEL_MEMORY_PAGE_RESERVE * SYSTEM_PAGE_SIZE;

        if (data_mem_region) {
            // log_info("Resizing kernel data memory by %d (requested %d)", new_size, size);
            if (!memory_management_region_resize(data_mem_region, new_size)) {
                return false;
            }
        }

        platform.memory.kernel_data.addr_end = platform.memory.kernel_data.addr_start + new_size;
        platform.memory.kernel_data.size = new_size;
    }
    return true;
}

/**
 * Kernel memory allocator. All memory allocated is filled by 0 by default.
 */
void* kalloc(size_t size)
{
    if (!ensure_free_memory(size)) {
        log_fatal("-*-*-*-*-*-*-*-*-* OutOfMemory-*-*-*-*-*-*-*-*-*-*-*-*");
        return NULL;
    }
    void *ptr = get_current_memory_ptr();

    // increase memory pointer
    current_mem_ptr += size;

    // log_trace("*** Allocated %d bytes (0x%x) at %p (%d KB). Used %d KB", size, size, ptr, (uintptr_t) ptr/1024, platform.memory.kernel_data.size/1024);
    memset(ptr, 0, size);
    return ptr;
}

/**
 * Release
 */
void kfree(void *addr)
{
    if (addr != NULL) {
        // release
    }
    // log_info("Releasing memory at %p (dummy). Used %d MB", addr, total_allocated/1024/1024);
    // TODO not implemented yet
}
