#include <kernel/logging.h>
#include <kernel/platform.h>
#include <kernel/page_allocator.h>
#include <kernel/multiboot2.h>
#include <kernel/bubackos.h>

static void __halt(void) {
	asm("hlt");
}

static void basic_logging(int log_level, const char* tag, const char* text) {
	logging(log_level, tag, text);
}

void intel_start(uint64_t magic, uintptr_t addr)
{
	// is necessary to initialize the terminal soon as possible to enable basic logging function
	terminal_initialize();
	log_set_level(LOG_DEBUG);
	// page_allocator_initialize();

	platform_t platform;
	// console
	platform.console.width = 80;
	platform.console.height = 25;
	platform.console.write_func = console__write;

	// logging
	platform.logging_func = &basic_logging;

	// cpu specific functions
	platform.halt = __halt;

	// memory configuration
	platform.heap_address = ((uintptr_t) __ADDR_KERNEL_END) + 1; // FIXME align

	uintptr_t stack_base = 0;
	asm ("movq %%rbp, %0; "
		:"=r"(stack_base)
	);
	log_info("Booting BubackOS");
	log_debug("	kernel loaded address=%p", (void*) __ADDR_KERNEL_START);
	log_debug("	kernel end address   =%p", (void*) __ADDR_KERNEL_END);
	log_debug("	stack base address   =%p", (void*) stack_base);

	if (multiboot_parser(magic, addr, &platform) == -1) {
		log_error("Try to use a bootloader with multiboot2 support");
		return;
	}

	bubackos_init(platform);
}
