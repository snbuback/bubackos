#include <system.h>
#include <string.h>
#include <kernel/logging.h>
#include <kernel/page_allocator.h>

void basic_logging(const char* tag, const char* text, char log_level) {
	printf("LOG %d %10s: %s\n", (unsigned) log_level, tag, text);
}

void halt(void) {
	asm("hlt");
}

void kernel_main(uint64_t magic, uintptr_t addr)
{
	uintptr_t stack_base = 0;
	asm ("movq %%rbp, %0; "
		:"=r"(stack_base)
	);

	console__initialize();
	console__clear();
	terminal_initialize();

	LOG_INFO("Booting BubackOS\n"
		"	kernel loaded address=%p\n"
		"	kernel end address   =%p\n"
		"	stack base address   =%p"
		, (void*) __ADDR_KERNEL_START, (void*) __ADDR_KERNEL_END, (void*) stack_base
	);

	platform_t platform;
	if (multiboot_parser(magic, addr, &platform) == -1) {
		LOG_ERROR("Try to use a bootloader with multiboot2 support");
		return;
	}

	// memory configuration
	// platform.total_memory = 1*1024*1024; // FIXME
	platform.heap_address = ((uintptr_t) __ADDR_KERNEL_END) + 1; // FIXME align

	// logging
	platform.logging_func = basic_logging;

	// console
	platform.console.width = 80;
	platform.console.height = 25;
	platform.console.write_func = console__write;

	// cpu specific functions
	platform.halt = halt;

	bubackos_init(&platform);
}
