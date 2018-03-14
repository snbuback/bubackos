#include <kernel/console.h>
#include <string.h>

const size_t console__width = 80;
const size_t console__height = 25;
static uint16_t* console_buffer = (uint16_t*) 0xB8000;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void console__initialize(void) {
	// nothing to do for now
}

void __console_clear(size_t row_start, size_t col_start, size_t row_end, size_t col_end) {
	for (size_t y = row_start; y < row_end; y++) {
		for (size_t x = col_start; x < col_end; x++) {
			const size_t index = y * console__width + x;
			console_buffer[index] = vga_entry('\0', 0);
		}
	}
}

void console__clear(void) {
	__console_clear(0, 0, console__height, console__width);
}

void console__write(const char* sequence, size_t length, uint8_t color, size_t row, size_t col) {
	const size_t index = row * console__width + col;
	for (size_t i = 0; i < length; i++) {
		console_buffer[index+i] = vga_entry(sequence[i], color);
	}
}

void console_scroll_up(size_t rows) {
	memcpy(console_buffer, &console_buffer[rows * console__width * sizeof(uint16_t)], (console__height-rows) * console__width * sizeof(uint16_t));
	__console_clear(console__height-rows, 0, console__height, console__width);
}
