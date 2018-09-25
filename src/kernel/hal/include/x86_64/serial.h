#ifndef _HAL_SERIAL_H_
#define _HAL_SERIAL_H_
#include <stdlib.h>

#define SERIAL_COM1         0x3F8
#define SERIAL_COM2         0x2F8
#define SERIAL_COM3         0x3E8
#define SERIAL_COM4         0x2E8

void serial_init();
void serial_write(const char* bytes, size_t size);

#endif