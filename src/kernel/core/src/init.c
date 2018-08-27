#include <core/init.h>
#include <logging.h>
#include <core/page_allocator.h>
#include <core/task_management.h>
#include <hal/configuration.h>
#include <core/elf.h>
#include <core/memory.h>
#include <core/module_loader.h>
#include <string.h>

void switch_kernel_pages()
{
    log_debug("Switching kernel pages...");
    memory_t* memory_handler = memory_management_get_kernel();
    native_pagetable_switch(memory_handler->pt);
}

// implicit argument platform_t platform
void bubackos_init() {

    logging_init();

    log_info("Booting BubackOS...");
    log_debug(" kernel loaded at %p with size %d KB", platform.memory.kernel.addr_start, 
        platform.memory.kernel.size/1024);
    log_debug(" kernel data address starting at %p", platform.memory.kernel_data.addr_start);

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

    memory_allocator_initialize();

    task_management_initialize();

    module_initialize(platform);

    // switch_kernel_pages();

    log_info("System ready");
}
