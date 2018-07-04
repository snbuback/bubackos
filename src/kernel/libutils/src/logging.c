#include <hal/console.h>
#include <logging.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libutils/utils.h>
#include <stdlib.h>

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