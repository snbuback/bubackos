#include <kernel/configuration.h>

// Logging with file kprintf("... %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)

#ifdef SYSTEM_DEBUG
  #define LOG_DEBUG(fmt, args...) kprintf("DEBUG %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)
#else
  #define LOG_DEBUG(...) /* Don't do anything */
#endif

#define LOG_INFO(fmt, args...) kprintf("INFO  %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)

#define LOG_WARNING(fmt, args...) kprintf("WARN  %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)

#define LOG_ERROR(fmt, args...) kprintf("ERROR %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)

void kprintf(const char *format, ...);
