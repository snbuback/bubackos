#include <string.h>
#include <logging.h>
#include <core/memory.h>

// TODO Replaces this dummy implementation
#define MEMORY_BUFFER   3*1024*1024
static unsigned char kernel_memory[MEMORY_BUFFER];
static size_t total_allocated = 0;

/**
 * Kernel memory allocator. All memory allocated is filled by 0 by default.
 */
void* malloc(size_t size)
{
    void *ptr = kernel_memory + total_allocated;
    total_allocated += size;
    if (total_allocated >= MEMORY_BUFFER) {
        log_fatal("-*-*-*-*-*-*-*-*-* OutOfMemory-*-*-*-*-*-*-*-*-*-*-*-*");
        return NULL;
    }
    log_trace("*** Allocated %d bytes (0x%x) at %p. Used %d KB", size, size, ptr, total_allocated/1024);
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
