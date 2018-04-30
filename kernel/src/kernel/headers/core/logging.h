#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) logging(LOG_TRACE, "", __VA_ARGS__)
#define log_debug(...) logging(LOG_DEBUG, "", __VA_ARGS__)
#define log_info(...)  logging(LOG_INFO,  "", __VA_ARGS__)
#define log_warn(...)  logging(LOG_WARN,  "", __VA_ARGS__)
#define log_error(...) logging(LOG_ERROR, "", __VA_ARGS__)
#define log_fatal(...) logging(LOG_FATAL, "", __VA_ARGS__)
#define LOGGING_MAX_LINE 80

void log_set_level(int level);
void log_set_quiet(int enable);

void logging_init();
void logging(int level, const char *tag, const char *fmt, ...);

#endif