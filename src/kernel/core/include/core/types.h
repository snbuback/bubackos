#ifndef _CORE_TYPES_H
#define _CORE_TYPES_H
#include <stdlib.h>
#include <stdint.h>

// Provides generic types

/**
 * Generic address structure
 */
typedef struct {
    uintptr_t addr_start;
    uintptr_t addr_end;
    size_t size;
} region_t;

#endif