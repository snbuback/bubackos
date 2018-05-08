#include <core/logging.h>
#include <core/memory_management.h>

// TODO Replaces this dummy implementation
#define MEMORY_BUFFER   2*1024*1024
static unsigned char kernel_memory[MEMORY_BUFFER];
static size_t total_allocated = 0;

/**
 * Kernel memory allocator
 */
void* kmem_alloc(size_t size)
{
    void *ptr = kernel_memory + total_allocated;
    total_allocated += size;
    // log_info("Allocated %d bytes (0x%x) at %p. Used %d MB", size, size, ptr, total_allocated/1024/1024);
    return ptr;
}

/**
 * Release
 */
void kmem_free(void *addr)
{
    log_info("Releasing memory at %p (dummy). Used %d MB", addr, total_allocated/1024/1024);
    // not implemented yet
}


