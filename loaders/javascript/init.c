#include <core/logging.h>
#include <javascript/js_engine.h>

// generated automatically during the build stage
extern void gen_load_all_modules(void);

void bubackos_init(platform_t platform) {
  log_info("Initializing platform independent modules");

  js_engine_initialize(&platform);
  gen_load_all_modules();

  js_engine_eval("OS.start()");

  log_info("System ready...");
};