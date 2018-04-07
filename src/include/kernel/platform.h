#ifndef __KERNEL_PLATFORM_H
#define __KERNEL_PLATFORM_H

#include <kernel/console.h>
#include <stdint.h>
#include <stdlib.h>

typedef size_t console_pos_t;
typedef uint8_t text_color_t;

typedef struct {
    console_pos_t width;
    console_pos_t height;
    void (*write_func)(const char* text, size_t size, text_color_t color, console_pos_t row, console_pos_t col);
    // missing read
} text_console_t;

typedef struct {
    size_t total_memory;
    uintptr_t heap_address;
    text_console_t console;
    void (*logging_func)(int log_level, const char* tag, const char* text);
    void (*halt)(void);
    // missing memory map
    // missing context switch
} platform_t;

void platform_initialize(platform_t platform_local);

#endif
