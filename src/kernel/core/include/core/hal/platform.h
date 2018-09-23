#ifndef __CORE_HAL_PLATFORM_H
#define __CORE_HAL_PLATFORM_H

// Defines the size of the pages of memory.
// The hardware implementation must support at least a multiple of this (ideally this size)
#define SYSTEM_PAGE_ALIGN                   12
#define SYSTEM_PAGE_SIZE                    (2 << (SYSTEM_PAGE_ALIGN-1))
#define SYSTEM_STACKSIZE                    32768
#define TASK_DEFAULT_STACK_SIZE             SYSTEM_PAGE_SIZE*10

#define REGION_ADDRESS_INCREMENT            (1*1024*1024)   // 1GB

#ifndef __ASSEMBLER__
#include <stdlib.h>
#include <algorithms/linkedlist.h>
#include <core/types.h>

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

/**
 * Returns the platform_t object.
 */
platform_t* get_platform_config();

#endif  // __ASSEMBLER__

#endif