#ifndef __KERNEL_CONSOLE_H
#define __KERNEL_CONSOLE_H
#include <stdint.h>
#include <stddef.h>

#define TAB_SIZE   4

// console
extern const size_t console__width;
extern const size_t console__height;
void console__clear(void);
void console__write(const char* sequence, size_t length, uint8_t color, size_t x, size_t y);
void console_scroll_up(size_t rows);

// terminal
void terminal_initialize(void);
void terminal__set_color(uint8_t fg, uint8_t bg);
void terminal__write(const char* data, size_t data_length);
void terminal__print(const char* data);
#endif
