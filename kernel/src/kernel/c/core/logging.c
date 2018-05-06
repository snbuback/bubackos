#include <hal/console.h>
#include <core/logging.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int level;
    int quiet;
} logging_config_t;

// The default values here are utilized by the HAL module during startup
static logging_config_t L = { .level = LOG_INFO, .quiet = 0 };

static const char* level_names[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

void log_set_level(int level)
{
    L.level = level;
}

void log_set_quiet(int enable)
{
    L.quiet = enable ? 1 : 0;
}

void logging(int level, const char* tag, const char* fmt, ...)
{
    if (level < L.level) {
        return;
    }

    if (!L.quiet) {
        char buf[LOGGING_MAX_LINE + 2];

        // print logging line header
        snprintf(buf, LOGGING_MAX_LINE, "%-5s %s: ", level_names[level], tag);

        // print the logging itself
        int size = strlen(buf);
        va_list args;
        va_start(args, fmt);
        size += vsnprintf(buf + size, LOGGING_MAX_LINE - size, fmt, args);
        va_end(args);

        int final_size = strlen(buf);
        if (final_size < size) {
            // register in the log there was a truncate
            buf[final_size-1] = '#';
        }
        buf[final_size] = '\n';
        buf[final_size + 1] = 0;

        console_print(buf);
    }
}

void logging_init() {
    log_set_level(LOG_DEBUG);
    log_set_quiet(0);
}