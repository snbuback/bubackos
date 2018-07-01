#include <hal/console.h>
#include <core/logging.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef struct {
    int level;
    int quiet;
} logging_config_t;

// The default values here are utilized by the HAL module during startup
static logging_config_t L = { .level = LOG_TRACE, .quiet = 0 };

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

int logging_format(char* buf, int level, const char *tag, const char *format, va_list args) {
        // print logging line header
        snprintf(buf, LOGGING_MAX_LINE + 1, "%s %s: ", level_names[level], tag);

        // print the logging itself
        int size = strlen(buf);
        size += vsnprintf(buf + size, LOGGING_MAX_LINE + 1 - size, format, args);
        return MIN(size, LOGGING_MAX_LINE);
}

void logging_output(char *log_line, size_t size)
{
    log_line[size] = '\n';
    log_line[size+1] = '\0';
    console_write(log_line, size+1);
}

void logging(int level, const char* tag, const char* fmt, ...)
{
    if (level < L.level) {
        return;
    }

    if (!L.quiet) {
        char buf[LOGGING_MAX_LINE + 2];  // + \n\0
        va_list args;
        va_start(args, fmt);
        int size = logging_format(buf, level, tag, fmt, args);
        va_end(args);

        logging_output(buf, size);
    }
}

void logging_init() {
    log_set_level(LOG_TRACE);
    log_set_quiet(0);
}

