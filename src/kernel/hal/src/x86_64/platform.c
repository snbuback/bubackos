#include <x86_64/serial.h>
#include <x86_64/platform.h>
#include <libutils/utils.h>

static platform_t platform;

/* defined by the linker */
extern uintptr_t __ADDR_KERNEL_START[];
extern uintptr_t __ADDR_KERNEL_END[];

bool platform_initialize()
{
	// fill platform_t object
	platform.memory.kernel.addr_start = (uintptr_t) __ADDR_KERNEL_START;
	platform.memory.kernel.addr_end = (uintptr_t) __ADDR_KERNEL_END;
	platform.memory.kernel.size = platform.memory.kernel.addr_end - platform.memory.kernel.addr_start;

	// kernel data starts on next page after kernel code.
	platform.memory.kernel_data.addr_start = ALIGN(platform.memory.kernel.addr_end, SYSTEM_PAGE_SIZE) + SYSTEM_PAGE_SIZE;
	platform.memory.kernel_data.addr_end = platform.memory.kernel_data.addr_start;
	platform.memory.kernel_data.size = 0;

	platform.memory.reserved_segments = linkedlist_create();
	platform.modules = linkedlist_create();
    return true;
}

platform_t* get_platform_config()
{
	return &platform;
}

void native_logging(const char* msg, size_t size)
{
    serial_write(msg, size);
}