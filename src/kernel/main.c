#include <system.h>
#include <string.h>
#include <kernel/logging.h>
#include <kernel/page_allocator.h>

void kernel_main(uint64_t magic, uint64_t *addr)
{

	console__initialize();

	console__clear();

	/* Initialize terminal interface */
	terminal_initialize();

	LOG_INFO("Booting at 0x%x", &kernel_main);

	page_allocator_initialize(128*1024*1024);

	mem_alloc_initialize();

	services_initialize();

	// SyscallInfo_t logging;
	// logging.name = "logging";
	// logging.call = (Syscall) log_info;
	//
	// servicehandler_t syscall_logging = services_register(logging);
	// services_call(syscall_logging, "\n**** testing first syscall ****\n");

	if (multiboot_parser(magic, addr) == -1) {
		LOG_ERROR("Try to use a bootloader with multiboot2 support");
		return;
	}

	LOG_DEBUG("linha 1 0x%x", malloc(5));
	LOG_DEBUG("linha 2 0x%x", sbrk(0));
	// LOG_DEBUG("linha 3");
}
