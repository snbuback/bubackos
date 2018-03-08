#include <system.h>
#include <string.h>

void log_info(char* str) {
	terminal__write(str, strlen(str));
}

void kernel_main(uint64_t magic, uint64_t *addr)
{

	console__initialize();

	console__clear();

	/* Initialize terminal interface */
	terminal_initialize();

	services_initialize();

	kprintf("Terminal Initialized qemu final\n");

	SyscallInfo_t logging;
	logging.name = "logging";
	logging.call = (Syscall) log_info;

	servicehandler_t syscall_logging = services_register(logging);
	services_call(syscall_logging, "\n**** testing first syscall ****\n");

	if (multiboot_parser(magic, addr) == -1) {
		kprintf("Try to use a bootloader with multiboot2 support");
		return;
	}

	log_info("Hello, kernel World!\n");
	kprintf("mbd=%x magic=%x\n", addr, magic);
	gdt_install();
	// kprintf("mem_lower=%d mem_upper=%d\n", mbd->mem_lower, mbd->mem_upper);
	log_info("Karol\n");
}
