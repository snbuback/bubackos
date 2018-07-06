#include <stdlib.h>
#include <stdbool.h>

void serial_init();
bool serial_received();
char serial_read();
void serial_write_char(char a);
void serial_write(const char* bytes, size_t size);
