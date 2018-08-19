#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <logging.h>
#include <libutils/utils.h>

typedef struct {
    int level;
    int quiet;
} logging_config_t;

// The default values here are utilized by the HAL module during startup
static logging_config_t L = { .level = LOG_TRACE, .quiet = 0 };

const char* LOGGING_LEVEL_NAMES[] = {
    "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"
};

void log_set_level(int level)
{
    L.level = level;
}

void log_set_quiet(int enable)
{
    L.quiet = enable ? 1 : 0;
}

static inline int logging_format(char* buf, const char *format, va_list args)
{
    int size = vsnprintf(buf, LOGGING_MAX_LINE, format, args);
    size = MIN(size, LOGGING_MAX_LINE);
    buf[size] = 0; // ensure NULL terminate string
    return size;
}

/**
 * Verify if the next message is equal to the lastest one.
 */
bool should_write(char *text, size_t text_size)
{
    // store the last message and avoid repeat it many times
    static char last_message[LOGGING_MAX_LINE + 1] = {0};
    static int repetitions = 0;

    if (strncmp(text, last_message, text_size) == 0 && repetitions < LOGGING_AGGREGATE_MESSAGES) {
        ++repetitions;
        return false;
    }

    if (repetitions > 1) {
        // write the number of repetitions
        char output[LOGGING_MAX_LINE + 1];  // + \0
        int size = snprintf(output, LOGGING_MAX_LINE+1, ">> last message repeat more %d times", repetitions-1);
        logging_write(LOG_INFO, output, size);
    }

    strncpy(last_message, text, LOGGING_MAX_LINE);
    repetitions = 1;
    return true;
}

void logging(int level, const char* fmt, ...)
{
    if (level < L.level) {
        return;
    }

    if (!L.quiet) {
        char text[LOGGING_MAX_LINE+1];
        va_list args;
        va_start(args, fmt);
        int size = logging_format(text, fmt, args);
        va_end(args);

        if (!should_write(text, size)) {
            return;
        }
        logging_write(level, text, size);
    }
}

void logging_init() {
    log_set_level(LOG_TRACE);
    log_set_quiet(0);
}

void __attribute__((weak)) logging_write(int level __attribute__((unused)), char* text __attribute__((unused)), size_t text_size __attribute__((unused)))
{
    // if there is no implementation
}