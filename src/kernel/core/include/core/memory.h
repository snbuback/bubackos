#ifndef _MEMORY_MANAGEMENT_H
#define _MEMORY_MANAGEMENT_H
#include <stdlib.h>
#include <stdbool.h>

#define NEW(type)				((type*) kalloc(sizeof(type)))
#define FREE(addr)				kfree(addr)

bool memory_allocator_initialize();

void* kalloc(size_t);
void kfree(void*);

#endif