#include <kernel/configuration.h>
#include <stdio.h>

// Logging with file printf("... %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##args)

#ifdef SYSTEM_DEBUG
  #define LOG_DEBUG(fmt, args...) printf("DEBUG " fmt "\n", ##args)
#else
  #define LOG_DEBUG(...) /* Don't do anything */
#endif

#define LOG_INFO(fmt, args...) printf("INFO  " fmt "\n", ##args)

#define LOG_WARNING(fmt, args...) printf("WARN  " fmt "\n", #args)

#define LOG_ERROR(fmt, args...) printf("ERROR " fmt "\n", ##args)

#define DEBUG_MEMORY