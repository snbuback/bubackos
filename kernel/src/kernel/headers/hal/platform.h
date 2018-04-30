#ifndef __HAL_PLATFORM_H
#define __HAL_PLATFORM_H

#include <stdint.h>
#include <stdlib.h>
#include <algorithms/queue.h>

typedef size_t console_pos_t;
typedef uint8_t text_color_t;

typedef struct {
    console_pos_t width;
    console_pos_t height;
    void (*write_func)(const char* text, size_t size, text_color_t color, console_pos_t row, console_pos_t col);
    // missing read
} text_console_t;

typedef struct {
    uintptr_t heap_address;
    size_t total_memory;
    // List of multiboot_memory_map_t
    Node *memory_segments; 
} memory_info_t;

typedef struct {
    memory_info_t memory_info;
    text_console_t console;
} platform_t;

#endif
