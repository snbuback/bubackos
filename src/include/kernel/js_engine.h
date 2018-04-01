#ifndef __KERNEL_JS_ENGINE_H
#define __KERNEL_JS_ENGINE_H
#include <system.h>

bool js_engine_initialize(platform_t *platform);
bool js_engine_module_load(const char* module_name, const char* source_code, size_t size);
bool js_engine_eval(const char* str_to_eval);

#endif
