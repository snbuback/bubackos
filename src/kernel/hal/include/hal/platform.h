#ifndef __HAL_PLATFORM_H
#define __HAL_PLATFORM_H
#include <stdlib.h>
#include <core/types.h>
#include <algorithms/linkedlist.h>

/**
 * Hardware Memory info
 */
typedef struct {
    size_t total_memory;
    region_t kernel;
    region_t kernel_data;
    linkedlist_t* reserved_segments; // region_t
} info_memory_info_t;

/**
 * Module
 */
typedef struct {
    char* param;
    region_t region;
} info_module_t;

typedef struct {
    info_memory_info_t memory;
    linkedlist_t* modules; // info_module_t
} platform_t;

extern platform_t platform;

#endif
