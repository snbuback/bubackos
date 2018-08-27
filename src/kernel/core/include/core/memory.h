#ifndef _MEMORY_MANAGEMENT_H
#define _MEMORY_MANAGEMENT_H
#include <stdlib.h>
#include <stdbool.h>

#define NEW(type)				((type*) malloc(sizeof(type)))
#define FREE(addr)				free(addr)

bool memory_allocator_initialize();

#endif