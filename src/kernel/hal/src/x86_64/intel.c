#define __TAG "==init=="
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <core/logging.h>
#include <hal/platform.h>
#include <core/init.h>
#include <core/page_allocator.h>
#include <hal/multiboot2.h>
#include <hal/console.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <hal/native_task.h>
#include <libutils/utils.h>

/* defined by the linker */
extern uintptr_t __ADDR_KERNEL_START[];
extern uintptr_t __ADDR_KERNEL_END[];

// TODO Remove this declaration
// TODO (Idea) Call first multiboot_parser and after intel_start with the platform object filled.
int multiboot_parser(uint64_t magic, uintptr_t addr, platform_t* platform);

// this reference should be allocate in the data section, to be available after this routine returns
platform_t platform;

void intel_start(uint64_t magic, uintptr_t addr)
{
	platform.memory.kernel.addr_start = (uintptr_t) __ADDR_KERNEL_START;
	platform.memory.kernel.addr_end = (uintptr_t) __ADDR_KERNEL_END;
	platform.memory.kernel.size = platform.memory.kernel.addr_end - platform.memory.kernel.addr_start;
	platform.memory.memory_segments = linkedlist_create();
	platform.modules = linkedlist_create();

	// is necessary to initialize the terminal soon as possible to enable basic logging function
	console_initialize();

	gdt_install();
	idt_initialize();

	// multiboot_parser fills memory information
	if (multiboot_parser(magic, addr, &platform) == -1) {
		log_error("Try to use a bootloader with multiboot2 support");
		return;
	}

	bubackos_init(&platform);

	idt_install();

    log_info("Intel System ready");
}

void logging_write(int level, char* text, size_t text_size __attribute__((unused)))
{
	#define CONSOLE_LINE 		80
    char output[CONSOLE_LINE + 1];  // + \0

    // print logging line header
    int size = snprintf(output, CONSOLE_LINE+1, "%s: %s\n", LOGGING_LEVEL_NAMES[level], text);
	size = MIN(size, CONSOLE_LINE);
    output[size] = '\0';
    console_write(output, size);
}
