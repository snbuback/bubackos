#ifndef __HAL_PLATFORM_H
#define __HAL_PLATFORM_H

#include <stdint.h>
#include <stdlib.h>
#include <algorithms/linkedlist.h>

// TODO remover this dependency
#include <hal/multiboot2.h>

typedef uint8_t text_color_t;

/**
 * Generic address structure
 */
typedef struct {
    uintptr_t addr_start;
    uintptr_t addr_end;
    size_t size;
} region_t;

/**
 * text console
 */
typedef struct {
    int width;
    int height;
    int pos_row;
    int pos_col;
} info_console_info_t;

/**
 * Hardware Memory info
 */
typedef struct {
    size_t total_memory;
    region_t kernel;
    linkedlist_t* memory_segments; // multiboot_memory_map_t
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
    info_console_info_t console;
    linkedlist_t* modules; // info_module_t
} platform_t;

#endif
