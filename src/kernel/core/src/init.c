#include <core/init.h>
#include <logging.h>
#include <core/page_allocator.h>
#include <hal/configuration.h>
#include <core/elf.h>
#include <core/memory.h>
#include <core/module_loader.h>
#include <core/scheduler/services.h>
#include <string.h>

__attribute__((noreturn)) void welcome_message()
{
    log_info("\n\n\n******* System ready! ********\n\n");
    asm volatile ("movq $1, %rdi; int $50");
    for(;;);
}

// implicit argument platform_t platform
void bubackos_init() {

    logging_init();

    log_info("Booting BubackOS...");
    log_debug(" kernel loaded at %p with size %d KB", platform.memory.kernel.addr_start, 
        platform.memory.kernel.size/1024);
    log_debug(" kernel data address starting at %p", platform.memory.kernel_data.addr_start);

    log_info("Platform data model (bytes): short=%d int=%d long=%d longlong=%d float=%d double=%d size_t=%d pointers=%d",
        sizeof(short),
        sizeof(int),
        sizeof(long),
        sizeof(long long),
        sizeof(float),
        sizeof(double),
        sizeof(size_t),
        sizeof(void*)
    );

    page_allocator_initialize(platform.memory.total_memory);

    // mark page of system segments
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(platform.memory.reserved_segments, &iter);
    region_t* region;
    while ((region = (region_t*) linkedlist_iter_next(&iter))) {
        page_allocator_mark_as_system(region->addr_start, region->size);
    }

    // mark pages of the kernel
    page_allocator_mark_as_system(platform.memory.kernel.addr_start, platform.memory.kernel.size);

    vmem_initialize();

    vmem_region_initialize();

    memory_allocator_initialize();

    scheduler_initialise();

    task_service_initialize();

    // say Welcome in another thread, to ensure the context switching is working properly.
    task_t* welcome_task = task_create("welcome", vmem_create());
    task_set_kernel_mode(welcome_task);
    task_run(welcome_task, (uintptr_t) &welcome_message);

    // initialise modules
    module_initialize(platform);
}
