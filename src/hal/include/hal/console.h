#ifndef __KERNEL_CONSOLE_H
#define __KERNEL_CONSOLE_H
#include <stdint.h>
#include <stdlib.h>

#define TAB_SIZE   4

void console_initialize(void);
void console_raw_write(const char* sequence, size_t length, uint8_t color, size_t x, size_t y);
void console_write(const char* data, size_t data_length);
void console_print(const char* data);
#endif
