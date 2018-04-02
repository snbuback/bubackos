#ifndef __KERNEL_SYSTEM_H
#define __KERNEL_SYSTEM_H

#include <kernel/gdt.h>
#include <kernel/memory_allocator.h>
#include <kernel/console.h>

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
    void (*logging_func)(const char* tag, const char* text, char log_level);
    void (*halt)(void);
    // missing memory map
    // missing context switch
} platform_t;

void bubackos_init(platform_t *platform);
int multiboot_parser(uint64_t magic, uintptr_t addr, platform_t* platform);

#endif
