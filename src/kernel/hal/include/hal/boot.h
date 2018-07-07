#ifndef _HAL_BOOT_H
#define _HAL_BOOT_H
#include <stdint.h>
#include <hal/platform.h>

void native_boot(uint64_t magic, uintptr_t addr);
bool multiboot_parser(uint64_t magic, uintptr_t addr, platform_t *platform);

#endif