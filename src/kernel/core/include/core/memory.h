#ifndef _MEMORY_MANAGEMENT_H
#define _MEMORY_MANAGEMENT_H
#include <stdlib.h>

#define NEW(type)				((type*) malloc(sizeof(type)))
#define FREE(addr)				free(addr)


#endif