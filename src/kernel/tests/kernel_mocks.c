// mock implementations
#include <kernel_test.h>
#include <stdarg.h>
#include <hal/platform.h>
#include <hal/native_pagging.h>
#include <hal/native_task.h>

// for memory mapping
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// by default the system have 1MB of memory
#ifndef SYSTEM_MEMORY_SIZE
#define SYSTEM_MEMORY_SIZE          ((size_t) 1*1024*1024)
#endif

#define SHM_MEMORY_NAME             "/kernel-data-mem"
#define KERNEL_START_MEM_ADDR       ((uintptr_t) 0x100000)

#define log_test(...)               logging(-1, __VA_ARGS__)

#define BOOT_FAIL(cond)             if (cond) { fail_initialisation(__LINE__); }

static void fail_initialisation(int line) {
    printf("Error initialising mock kernel. %s:%d", __FILE__, line);
    TEST_FAIL_MESSAGE("Error initialising mock kernel");

    // in case we are not in a test case, fall back to exit
    exit(1);
}

// Most of functions here are weak, since the could be overriden if it is necessary

// by default logging just write on stdout
void __attribute__((weak)) logging(int level, const char *fmt, ...)
{
    printf(">>>%d: ", level);
    va_list args;
    va_start(args, fmt);

    int size = vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);  // disable buffering
}

/**
 * Alocates a block of memory at specific address and size, simulating the physical memory
 */
static uintptr_t allocate_memory(uintptr_t origin_addr, size_t size)
{
    int fd = shm_open(SHM_MEMORY_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    BOOT_FAIL(fd == -1);

    int res = ftruncate(fd, size);
    BOOT_FAIL(res == -1);

    uintptr_t addr = (uintptr_t) mmap((void*) origin_addr, size, PROT_WRITE, MAP_SHARED, fd, 0);
    BOOT_FAIL(origin_addr != addr);
    return addr;
}

// this simulates the work done by the boot platform
platform_t platform;

__attribute__ ((constructor)) void boot(void)
{
	platform.memory.kernel.addr_start = 0x2000;
	platform.memory.kernel.addr_end = 0xA000;
	platform.memory.kernel.size = platform.memory.kernel.addr_end - platform.memory.kernel.addr_start;

	// kernel data starts on next page after kernel code.
	platform.memory.kernel_data.addr_start = allocate_memory(KERNEL_START_MEM_ADDR, SYSTEM_MEMORY_SIZE);
	platform.memory.kernel_data.addr_end = platform.memory.kernel_data.addr_start;
	platform.memory.kernel_data.size = 0;

    // these are initialised with empty linkedlist, but for test they are initialised with empty
	platform.memory.reserved_segments = NULL;
	platform.modules = NULL;
}

// by default memory function just increment memory usage
void* __attribute__((weak)) kalloc(size_t size)
{
    // log_test("kalloc called with size %d", size);
    uintptr_t addr = platform.memory.kernel_data.addr_end;

    // align memory address
    uintptr_t aligned_addr = ALIGN_NEXT(addr, sizeof(uintptr_t));
    size += aligned_addr - addr;

    platform.memory.kernel_data.addr_end += size;
    platform.memory.kernel_data.size += size;
    return (void*) aligned_addr;
}

void __attribute__((weak)) kfree(void* ptr)
{
    // log_test("kfree called with addr %p", ptr);
}

native_page_table_t* __attribute__((weak)) native_pagetable_create()
{
    log_test("native_pagetable_create called");
    native_page_table_t* pt = kalloc(sizeof(native_page_table_t));
    TEST_ASSERT_NOT_NULL(pt);
    return pt;
}

void __attribute__((weak)) native_pagetable_set(native_page_table_t* pt, page_map_entry_t entry)
{
    // log_test("native_pagetable_set called");
    TEST_ASSERT_NOT_NULL(pt);
}

void __attribute__((weak)) native_page_table_flush()
{
    log_test("native_page_table_flush called");
}

void __attribute__((weak)) native_pagetable_switch(native_page_table_t* pt)
{
    log_test("native_pagetable_switch called");
}

void __attribute__((weak)) hal_switch_task(native_task_t *task)
{
    log_test("hal_switch_task called");
    for (;;) {
        log_test("No-return!!!!");
    }
}

void __attribute__((weak)) hal_sleep(void)
{
    log_test("hal_sleep called");
    for (;;) {
        log_test("No-return!!!!");
    }
}

void __attribute__((weak)) hal_create_native_task(native_task_t *task, uintptr_t code, uintptr_t stack, int permission_mode, uintptr_t userdata)
{
    log_test("hal_create_native_task called");
}