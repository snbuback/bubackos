#include <logging.h>
#include <core/init.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <x86_64/platform.h>
#include <common/multiboot2.h>
#include <x86_64/serial.h>

void native_boot(uint64_t magic, uintptr_t addr)
{
	// initialize the logging system
	serial_init();
	platform_initialize();

	// multiboot_parser fills memory information
	if (!multiboot2_parser(magic, addr)) {
		log_error("Try to use a bootloader with multiboot2 support");
		return;
	}

	gdt_initialize();
	idt_initialize();
    log_info("Native System ready");

	kernel_main();
}
