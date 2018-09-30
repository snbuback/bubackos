#include <logging.h>
#include <stdio.h>
#include <libutils/utils.h>
#include <core/hal/platform.h>

void logging_write(int level, char* text, size_t text_size __attribute__((unused)))
{
    char output[LOGGING_MAX_LINE + 1];  // + \0

    // print logging line header
    int size = snprintf(output, LOGGING_MAX_LINE+1, "%s: %s\n", LOGGING_LEVEL_NAMES[level], text);
	size = MIN(size, LOGGING_MAX_LINE);
    output[size] = '\0';

	native_logging(output, size);
}

