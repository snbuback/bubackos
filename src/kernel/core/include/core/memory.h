#ifndef _MEMORY_MANAGEMENT_H
#define _MEMORY_MANAGEMENT_H
#include <stdlib.h>

#define NEW(type)				((type*) kmem_alloc(sizeof(type)))
#define FREE(addr)				kmem_free(addr)

/**
 * Kernel memory allocator
 */
void* kmem_alloc(size_t size);

/**
 * Release
 */
void kmem_free(void *addr);

#endif