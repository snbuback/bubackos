#include <kernel/console.h>

volatile size_t terminal_row;
volatile size_t terminal_column;
volatile uint8_t terminal_color;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal__set_color(15, 0);
}

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
	return fg | bg << 4;
}

void terminal__set_color(uint8_t fg, uint8_t bg) {
	terminal_color = vga_entry_color(fg, bg);
}

static inline void terminal_writesequence(const char* buffer_start, size_t length) {
	console__write(buffer_start, length, terminal_color, terminal_row, terminal_column);
	terminal_column += length;
}

static inline void newline() {
	terminal_column = 0;
	terminal_row++;
	if (terminal_row > 24) {
		terminal_row = 1;
		console__write(">>>>", 4, terminal_color, 0, 0);
	}
}

void terminal__write(const char* data, size_t data_length) {
	// TODO implement automatic new-line and scroll
	if (data == 0 || data_length == 0) {
		return;
	}

	char* buffer_start = (char*) data;
	size_t i;
	for (i=0; i < data_length; i++) {
		switch (data[i]) {
			case '\n':
				terminal_writesequence(buffer_start, data + i - buffer_start);
				buffer_start = (char*) data+i+1;
				newline();
				break;
			case '\r':
				terminal_writesequence(buffer_start, data + i - buffer_start);
				terminal_column = 0;
				buffer_start = (char*) data+i+1;
				break;
		}
	}
	terminal_writesequence(buffer_start, data + i - buffer_start);
}
