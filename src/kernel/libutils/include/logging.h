#ifndef LOG_H
#define LOG_H
#include <stdlib.h>

#define LOGGING_MAX_LINE                255
#define LOGGING_AGGREGATE_MESSAGES      30  // number of repetitive message to avoid rewrite

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
extern const char* LOGGING_LEVEL_NAMES[];

#define log_trace(...) logging(LOG_TRACE, __VA_ARGS__)
#define log_debug(...) logging(LOG_DEBUG, __VA_ARGS__)
#define log_info(...)  logging(LOG_INFO,  __VA_ARGS__)
#define log_warn(...)  logging(LOG_WARN,  __VA_ARGS__)
#define log_error(...) logging(LOG_ERROR, __VA_ARGS__)
#define log_fatal(...) logging(LOG_FATAL, __VA_ARGS__)

void log_set_level(int level);
void log_set_quiet(int enable);

void logging_init();
void logging(int level, const char *fmt, ...);
void logging_write(int level, char* log_line, size_t size);

#endif