#include <kernel/logging.h>
#include <kernel/platform.h>
#include <kernel/page_allocator.h>
#include <kernel/multiboot2.h>
#include <kernel/bubackos.h>
#include <kernel/console.h>

platform_t platform;

static void __halt(void) {
	asm("hlt");
}

static void platform_logging(int log_level, const char* tag, const char* text) {
	logging(log_level, tag, text);
}

void intel_start(uint64_t magic, uintptr_t addr)
{
	// is necessary to initialize the terminal soon as possible to enable basic logging function
	console_initialize();
	log_set_level(LOG_TRACE);

	// console
	platform.console.width = 80;
	platform.console.height = 25;
	platform.console.write_func = console_raw_write;

	// logging
	platform.logging_func = &platform_logging;

	// cpu specific functions
	platform.halt = __halt;

	platform.memory_info.heap_address = ((uintptr_t) __ADDR_KERNEL_END) + 1; // FIXME align

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

	page_allocator_initialize(platform.memory_info.total_memory);

	// mark page of system segments
	Node *next_mmap = platform.memory_info.memory_segments;
	while (next_mmap) {
		multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) next_mmap->val;
		page_allocator_mark_as_system(mmap->addr, mmap->len);
		next_mmap = next_mmap->next;
	}

	// mark pages of the kernel
	page_allocator_mark_as_system((uintptr_t) __ADDR_KERNEL_START, (size_t) __ADDR_KERNEL_END - (size_t) __ADDR_KERNEL_START);

	// reinitialize the head to the next page available
	// FIXME Pagination needs work to implement this


	bubackos_init(platform);
}
