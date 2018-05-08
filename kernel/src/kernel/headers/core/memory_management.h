#ifndef _MEMORY_MANAGEMENT_H
#define _MEMORY_MANAGEMENT_H

#include <stdlib.h>

/**
 * Kernel memory allocator
 */
void* kmem_alloc(size_t size);

/**
 * Release
 */
void kmem_free(void *addr);

#endif