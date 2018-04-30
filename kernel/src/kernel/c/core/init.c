#include <core/init.h>
#include <core/logging.h>
#include <core/page_allocator.h>

// FIXME
#include <hal/multiboot2.h>

platform_t platform;

void bubackos_init(platform_t platform) {

    logging_init();

	log_info("Booting BubackOS...");
	log_debug("	kernel loaded address=%p", (void*) __ADDR_KERNEL_START);
	log_debug("	kernel end address   =%p", (void*) __ADDR_KERNEL_END);

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

    log_info("System ready");

}