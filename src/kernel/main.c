#include <system.h>
#include <string.h>
#include <kernel/logging.h>

void kernel_main(uint64_t magic, uint64_t *addr)
{

	console__initialize();

	console__clear();

	/* Initialize terminal interface */
	terminal_initialize();

	LOG_INFO("Booting at 0x%x", &kernel_main);

	services_initialize();

	LOG_DEBUG("Terminal Initialized");

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

	LOG_DEBUG("linha 1");
	LOG_DEBUG("linha 2");
	// LOG_DEBUG("linha 3");
}
