#include <string.h>
#include <logging.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hal/platform.h>
#include <core/memory_management.h>
#include <core/page_allocator.h>

#define KERNEL_MEMORY_PAGE_RESERVE       5

static void* current_mem_ptr = NULL;
static bool initialised = false;

bool memory_allocator_initialize()
{
    if (!kernel_data_region) {
        log_warn("Kernel data region not found");
        return false;
    }

    // if during the memory allocation the kernel data memory grows more than allocated, there is an error
    size_t allocated_size = memory_management_region_current_size(kernel_data_region);
    if (platform.memory.kernel_data.size > allocated_size) {
        log_warn("Memory grows more than the allocated: total %d > allocated %d", platform.memory.kernel_data.size, allocated_size);
        return false;
    }
    initialised = true;
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

        if (initialised) {
            // log_info("Resizing kernel data memory by %d (requested %d)", new_size, size);
            if (!memory_management_region_resize(kernel_data_region, new_size)) {
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
