#include <core/init.h>
#include <core/logging.h>
#include <core/page_allocator.h>
#include <core/task_management.h>
#include <hal/hal.h>
#include <core/configuration.h>
#include <stdbool.h>

// FIXME
#include <hal/multiboot2.h>

/* defined by the linker */
extern uintptr_t __ADDR_KERNEL_START[];
extern uintptr_t __ADDR_KERNEL_END[];

platform_t platform;
extern int user_task1;
extern int user_task2;
extern int user_task3;

static void fill_kernel_pages(native_page_table_t* native_page_table_t)
{
    // TODO kernel pages should have a common pointer and be updated always when a new memory allocation happens
    // map all kernel page

    for (uintptr_t addr = (uintptr_t) __ADDR_KERNEL_START; addr <= (uintptr_t) __ADDR_KERNEL_END; addr += SYSTEM_PAGE_SIZE) {
        hal_page_table_add_mapping(native_page_table_t, addr, addr, true, true, true);
    }

    // fill video memory
    hal_page_table_add_mapping(native_page_table_t, 0xB8000, 0xB8000, true, true, true);
}

void user_tasks() {
    log_info("creating dummy tasks");

    native_page_table_t* pt = hal_page_table_create_mapping();
    fill_kernel_pages(pt);

    task_id_t task1 = task_create("user1", pt);
    task_start(task1, (uintptr_t) &user_task1);

    task_id_t task2 = task_create("user2", pt);
    task_start(task2, (uintptr_t) &user_task2);

    task_id_t task3 = task_create("user3", pt);
    task_start(task3, (uintptr_t) &user_task3);
}

void bubackos_init(platform_t platform) {

    logging_init();

    log_info("Booting BubackOS...");
    log_debug(" kernel loaded address=%p", (void*) __ADDR_KERNEL_START);
    log_debug(" kernel end address   =%p", (void*) __ADDR_KERNEL_END);

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

    task_management_initialize();
    
    user_tasks();
    log_info("System ready");
}
