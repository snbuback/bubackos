#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <logging.h>
#include <core/hal/platform.h>
#include <core/init.h>
#include <core/pmem/services.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <hal/native_task.h>
#include <hal/native_logging.h>
#include <core/hal/platform.h>
#include <libutils/utils.h>
#include <common/multiboot2.h>

/* defined by the linker */
extern uintptr_t __ADDR_KERNEL_START[];
extern uintptr_t __ADDR_KERNEL_END[];

static platform_t platform;

platform_t* get_platform_config()
{
	return &platform;
}

void native_boot(uint64_t magic, uintptr_t addr)
{
	// initialize the logging system
	native_logging_init();

	platform.memory.kernel.addr_start = (uintptr_t) __ADDR_KERNEL_START;
	platform.memory.kernel.addr_end = (uintptr_t) __ADDR_KERNEL_END;
	platform.memory.kernel.size = platform.memory.kernel.addr_end - platform.memory.kernel.addr_start;

	// kernel data starts on next page after kernel code.
	platform.memory.kernel_data.addr_start = ALIGN(platform.memory.kernel.addr_end, SYSTEM_PAGE_SIZE) + SYSTEM_PAGE_SIZE;
	platform.memory.kernel_data.addr_end = platform.memory.kernel_data.addr_start;
	platform.memory.kernel_data.size = 0;

	platform.memory.reserved_segments = linkedlist_create();
	platform.modules = linkedlist_create();

	// multiboot_parser fills memory information
	if (!multiboot2_parser(magic, addr)) {
		log_error("Try to use a bootloader with multiboot2 support");
		return;
	}

	gdt_install();
	idt_initialize();

	idt_install();

	bubackos_init(&platform);

    log_info("Native System ready");
}
