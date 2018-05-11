#include <string.h>
#include <stdlib.h>
#include <core/logging.h>
#include <hal/platform.h>
#include <core/init.h>
#include <core/page_allocator.h>
#include <hal/multiboot2.h>
#include <hal/console.h>
#include <hal/gdt.h>
#include <hal/idt.h>
#include <hal/native_task.h>

void intel_start(uint64_t magic, uintptr_t addr)
{
	platform_t platform;

	// is necessary to initialize the terminal soon as possible to enable basic logging function
	console_initialize();
	platform.console.width = 80;
	platform.console.height = 25;

	gdt_install();
	idt_initialize();

	// multiboot_parser fills memory information
	if (multiboot_parser(magic, addr, &platform) == -1) {
		log_error("Try to use a bootloader with multiboot2 support");
		return;
	}

	bubackos_init(platform);

	idt_install();

    log_info("Intel System ready");
}
