#include <system.h>
#include <kernel/logging.h>
#include <kernel/js_engine.h>

// generated automatically during the build stage
extern void gen_load_all_modules(void);

void bubackos_init(platform_t *platform) {
  platform->logging_func("init", "Initializing platform independent modules", LOG_LEVEL_INFO);

  js_engine_initialize(platform);
  gen_load_all_modules();

  js_engine_eval("OS.start()");

  platform->logging_func("init", "shuting down", LOG_LEVEL_INFO);
  platform->halt();

};