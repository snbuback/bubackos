#include <core/init.h>
#include <logging.h>
#include <core/page_allocator.h>
#include <core/task_management.h>
#include <hal/configuration.h>
#include <core/elf.h>
#include <core/memory.h>
#include <core/module_loader.h>
#include <string.h>

// TODO change platform to an argument. After the kernel initialize doesn't make sense keep this value (check how to implement memory allocation first)
extern platform_t platform;

void switch_kernel_pages()
{
    log_debug("Switching kernel pages...");
    native_pagetable_dump(NULL);
    memory_t* memory_handler = memory_management_create();
    // memory_region_t* region = memory_management_region_create(memory_handler, 0x0, 0, true, true, true);
    // fill_kernel_pages(region);
    native_pagetable_switch(memory_handler->pt);
}

// implicit argument platform_t platform
void bubackos_init() {

    logging_init();

    log_info("Booting BubackOS...");
    log_debug(" kernel loaded address=%p", platform.memory.kernel.addr_start);
    log_debug(" kernel end address   =%p", platform.memory.kernel.addr_end);

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

    memory_management_initialize();

    task_management_initialize();

    switch_kernel_pages();

    module_initialize(platform);

    log_info("System ready");
}
