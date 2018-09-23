#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <logging.h>
#include <core/hal/platform.h>
#include <core/init.h>
#include <core/page_allocator.h>
#include <hal/boot.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <hal/native_task.h>
#include <hal/native_logging.h>
#include <core/hal/platform.h>
#include <libutils/utils.h>

/* defined by the linker */
extern uintptr_t __ADDR_KERNEL_START[];
extern uintptr_t __ADDR_KERNEL_END[];

// this reference should be allocate in the data section, to be available after this routine returns
static platform_t platform;

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
	if (!multiboot_parser(magic, addr, &platform)) {
		log_error("Try to use a bootloader with multiboot2 support");
		return;
	}

	gdt_install();
	idt_initialize();

	bubackos_init(&platform);

	idt_install();

    log_info("Intel System ready\n\n");
}

platform_t* get_platform_config()
{
	return &platform;
}
