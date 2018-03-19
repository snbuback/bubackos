#include <kernel/configuration.h>
#include <stdio.h>

// Logging with file printf("... %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)

#ifdef SYSTEM_DEBUG
  #define LOG_DEBUG(fmt, args...) printf("DEBUG %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)
#else
  #define LOG_DEBUG(...) /* Don't do anything */
#endif

#define LOG_INFO(fmt, args...) printf("INFO  %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)

#define LOG_WARNING(fmt, args...) printf("WARN  %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)

#define LOG_ERROR(fmt, args...) printf("ERROR %s:%d:\t" fmt "\n", __func__, __LINE__, ##args)

