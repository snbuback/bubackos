#include <kernel/console.h>

const size_t console__width = 80;
const size_t console__height = 25;
static volatile uint16_t* console_buffer = (uint16_t*) 0xB8000;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void console__initialize(void) {
	// nothing to do for now
}

void console__clear(void) {
	for (size_t y = 0; y < console__height; y++) {
		for (size_t x = 0; x < console__width; x++) {
			const size_t index = y * console__width + x;
			console_buffer[index] = vga_entry('\0', 0);
		}
	}
}

void console__write(const char* sequence, size_t length, uint8_t color, size_t row, size_t col) {
	// TODO Avoid text leak to the next line
	const size_t index = row * console__width + col;
	for (size_t i = 0; i < length; i++) {
		console_buffer[index+i] = vga_entry(sequence[i], color);
	}
}
