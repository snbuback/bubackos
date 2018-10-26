#include <core/init.h>
#include <logging.h>
#include <core/pmem/services.h>
#include <core/hal/platform.h>
#include <core/alloc.h>
#include <core/loader/module_loader.h>
#include <core/scheduler/services.h>
#include <core/syscall/services.h>
#include <string.h>

static void display_boot_info()
{
    platform_t* platform = get_platform_config();
    size_t code = platform->memory.kernel.size;
    size_t data = platform->memory.kernel_data.size;
    log_info("Memory allocation: code %d KB / data %d KB / total %d KB", 
        code/1024,
        data/1024,
        (code+data)/1024);
}

void kernel_main() {

    platform_t* platform = get_platform_config();

    logging_init();

    log_info("Booting BubackOS...");
    log_debug(" kernel loaded at %p with size %d KB", platform->memory.kernel.addr_start, 
        platform->memory.kernel.size/1024);
    log_debug(" kernel data address starting at %p", platform->memory.kernel_data.addr_start);

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

    pmem_initialize(platform->memory.total_memory);

    // mark page of system segments
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(platform->memory.reserved_segments, &iter);
    region_t* region;
    while ((region = (region_t*) linkedlist_iter_next(&iter))) {
        pmem_set_as_system(region->addr_start, region->size);
    }

    // mark pages of the kernel
    pmem_set_as_system(platform->memory.kernel.addr_start, platform->memory.kernel.size);

    vmem_initialize();

    vmem_region_initialize();

    memory_allocator_initialize();

    scheduler_initialise();

    task_service_initialize();

    syscall_initialize();

    // initialise modules
    module_initialize(platform);
    
    display_boot_info();
}
