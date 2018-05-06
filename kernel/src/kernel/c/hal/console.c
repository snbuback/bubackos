// Very basic terminal functions
#include <stdlib.h>
#include <string.h>
#include <hal/console.h>

static volatile size_t terminal_row;
static volatile size_t terminal_column;
static volatile uint8_t terminal_color;

const size_t console__width = 80;
const size_t console__height = 25;
static uint16_t* console_buffer = (uint16_t*) 0xB8000;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
	return fg | bg << 4;
}

static inline int min(int a, int b)
{
	if (a<b) return a;
	return b;
};

void __console_clear(size_t row_start, size_t col_start, size_t row_end, size_t col_end) {
	for (size_t y = row_start; y < row_end; y++) {
		for (size_t x = col_start; x < col_end; x++) {
			const size_t index = y * console__width + x;
			console_buffer[index] = vga_entry('\0', 0);
		}
	}
}

static void console_clear(void) {
	__console_clear(0, 0, console__height, console__width);
}

void console_raw_write(const char* sequence, size_t length, uint8_t color, size_t row, size_t col) {
	const size_t index = row * console__width + col;
	for (size_t i = 0; i < length; i++) {
		console_buffer[index+i] = vga_entry(sequence[i], color);
	}
}

void console_initialize(void) {
	console_clear();
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(15, 0);
}

// Terminal functions

void console_scroll_up(size_t rows) {
	memcpy(console_buffer, &console_buffer[rows * console__width * sizeof(uint16_t)], (console__height-rows) * console__width * sizeof(uint16_t));
	__console_clear(console__height-rows, 0, console__height, console__width);
}

static void need_scroll() {
	if (terminal_row > console__height) {
		console_scroll_up(1);
		terminal_row = console__height-1;
	}
}

static inline void newline() {
	terminal_column = 0;
	++terminal_row;
	need_scroll();
}

static inline void terminal_writesequence(const char* buffer_start, size_t length) {
	size_t written = 0;
	do {
		int chars_to_write = min(console__width - terminal_column, length);
		console_raw_write(buffer_start+written, chars_to_write, terminal_color, terminal_row, terminal_column);
		terminal_column += chars_to_write;
		written += chars_to_write;
		if (terminal_column >= console__width) {
			newline();
		}
	} while (written < length);
}

void console_write(const char* data, size_t data_length) {
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
			case '\t':
				terminal_writesequence(buffer_start, data + i - buffer_start);
				terminal_column = ((terminal_column/TAB_SIZE)+1)*TAB_SIZE;
				buffer_start = (char*) data+i+1;
				break;
		}
	}
	terminal_writesequence(buffer_start, data + i - buffer_start);
}

void console_print(const char* data)
{
	console_write(data, strlen(data));
}

