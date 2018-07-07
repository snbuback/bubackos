#include <stdio.h>
#include <logging.h>
#include <hal/native_logging.h>
#include <x86_64/serial.h>
#include <libutils/utils.h>

/**
 * Logging write in the serial port
 */
void native_logging_init() {
    serial_init();

    // always starts the logging with a new line
    serial_write("\n\n", 2);
}

void logging_write(int level, char* text, size_t text_size __attribute__((unused)))
{
    char output[LOGGING_MAX_LINE + 1];  // + \0

    // print logging line header
    int size = snprintf(output, LOGGING_MAX_LINE+1, "%s: %s\n", LOGGING_LEVEL_NAMES[level], text);
	size = MIN(size, LOGGING_MAX_LINE);
    output[size] = '\0';

	serial_write(output, size);
}
