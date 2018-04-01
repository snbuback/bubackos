#include <system.h>
#include <kernel/logging.h>
#include <kernel/js_engine.h>

void bubackos_init(platform_t *platform) {
  platform->logging_func("init", "Initializing platform independent modules", LOG_LEVEL_INFO);

  js_engine_initialize(platform);

  start_cpu0();

  platform->logging_func("init", "shuting down", LOG_LEVEL_INFO);
  platform->halt();

} __attribute__ ((noreturn));;