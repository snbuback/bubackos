#include <stdlib.h>
#include <x86_64/intel.h>
#include <x86_64/serial.h>

// logging serial PORT
#define PORT        SERIAL_COM1

void serial_init()
{
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int is_transmit_empty()
{
   return inb(PORT + 5) & 0x20;
}
 
static inline void serial_write_char(char a)
{
   while (is_transmit_empty() == 0);
 
   outb(PORT, a);
}

void serial_write(const char* bytes, size_t size)
{
    while (size-- > 0) {
        serial_write_char(*bytes++);
    }
}