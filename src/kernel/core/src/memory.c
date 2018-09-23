#include <string.h>
#include <logging.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hal/platform.h>
#include <core/page_allocator.h>
#include <core/vmem/services.h>
#include <hal/configuration.h>

#define KERNEL_MEMORY_PAGE_RESERVE       3*SYSTEM_PAGE_SIZE

static void* current_mem_ptr = NULL;
bool mem_allocator_initialised = false;

static vmem_region_t* kernel_code_vmem_region;
vmem_region_t* kernel_data_vmem_region;

static bool vmem_map_current_kernel_memory_address()
{
    // code
    kernel_code_vmem_region = vmem_region_create(
        vmem_get_kernel(),
        "kernel-code",
        platform.memory.kernel.addr_start,
        0,
        false,
        true,
        true
    );

    if (!vmem_region_map_physical_address(
        kernel_code_vmem_region,
        platform.memory.kernel.addr_start,
        ALIGN_NEXT(platform.memory.kernel.size, SYSTEM_PAGE_SIZE))) {
        return false;
    }

    // data
    // TODO Since hal_switch_task uses pushq it is required execution permission on kernel memory
    uintptr_t data_addr = platform.memory.kernel_data.addr_start;
    kernel_data_vmem_region = vmem_region_create(
        vmem_get_kernel(),
        "kernel-data",
        data_addr,
        0,
        false,
        true,
        true
    );

    // for data, always allocates more 1 page to ensure any allocation before the memory allocator module
    // initialise still fits in the region mapped.
    size_t new_size = ALIGN_NEXT(platform.memory.kernel_data.size + 100*SYSTEM_PAGE_SIZE, SYSTEM_PAGE_SIZE);
    if (!vmem_region_map_physical_address(
        kernel_data_vmem_region,
        platform.memory.kernel_data.addr_start,
        new_size)
    ) {
        platform.memory.kernel_data.size = new_size;
        return false;
    }
    return true;
}

bool memory_allocator_initialize()
{
    if (!vmem_map_current_kernel_memory_address()) {
        return false;
    }

    // if during the memory allocation the kernel data memory grows more than allocated, there is an error
    size_t allocated_size = vmem_region_current_size(kernel_data_vmem_region);
    if (platform.memory.kernel_data.size > allocated_size) {
        log_warn("Memory grows more than the allocated: total %d > allocated %d", platform.memory.kernel_data.size, allocated_size);
        return false;
    }
    mem_allocator_initialised = true;

    log_info("Virtual Memory initialised. Switching to kernel pages.");
    native_pagetable_switch(vmem_get_kernel()->pt);
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
 * To avoid a recursion here, since requires memory to expand memory, I marked memory as expanded before.
 */
static bool ensure_free_memory(size_t size)
{
    // since it is required memory to expand memory, I always need to ensure there is a KERNEL_MEMORY_PAGE_RESERVE size of memory
    size_t minimum_required_size = (uintptr_t) get_current_memory_ptr() + size + KERNEL_MEMORY_PAGE_RESERVE - platform.memory.kernel_data.addr_start;
    size_t current_size = kernel_data_vmem_region ? kernel_data_vmem_region->allocated_size : platform.memory.kernel_data.size; // take into account current_size

    if (minimum_required_size > current_size) {
        // increments memory 2 * KERNEL_MEMORY_PAGE_RESERVE
        size_t new_size = ALIGN_NEXT(minimum_required_size + KERNEL_MEMORY_PAGE_RESERVE, SYSTEM_PAGE_SIZE);
        platform.memory.kernel_data.addr_end = platform.memory.kernel_data.addr_start + new_size;
        platform.memory.kernel_data.size = new_size;

        if (mem_allocator_initialised) {
            log_info("Resizing kernel data memory by +%d KB. New size is %d KB. Allocated %d KB. Current requested %d bytes.",
                (new_size - current_size)/1024,
                new_size/1024,
                kernel_data_vmem_region->allocated_size/1024,
                size);
            if (!vmem_region_resize(kernel_data_vmem_region, new_size)) {
                log_warn("Due to the lack of memory kernel data pointer are out-of-sync with the kernel memory.");
                return false;
            }
        }
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
