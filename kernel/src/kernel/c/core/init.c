#include <core/init.h>
#include <core/logging.h>
#include <core/page_allocator.h>
#include <core/task_management.h>
#include <hal/hal.h>
#include <core/configuration.h>
#include <loader/elf.h>
#include <core/memory.h>

extern int user_task1;
extern int user_task2;
extern int user_task3;

// TODO change platform to an argument. After the kernel initialize doesn't make sense keep this value (check how to implement memory allocation first)
extern platform_t platform;

static void fill_kernel_pages(native_page_table_t* native_page_table_t)
{
    // TODO kernel pages should have a common pointer and be updated always when a new memory allocation happens
    // map all memory to kernel
    for (uintptr_t addr = platform.memory.kernel.addr_start; addr <= platform.memory.kernel.addr_end; addr += SYSTEM_PAGE_SIZE) {
        hal_page_table_add_mapping(native_page_table_t, addr, addr, true, true, true);
    }

    // fill video memory
    hal_page_table_add_mapping(native_page_table_t, 0xB8000, 0xB8000, true, true, true);
}

void allocate_program_header(elf_program_header_t* ph, native_page_table_t* pt)
{
    size_t allocated = 0;
    uintptr_t vaddr = ph->vaddr;
    uintptr_t paddr = (uintptr_t) ph->offset;
    while (allocated < ph->mem_size) {
        // TODO don't allocate new memory. Use existing memory. Fixing this
        allocated += SYSTEM_PAGE_SIZE;
        hal_page_table_add_mapping(pt, vaddr, paddr, true, true, true);
        vaddr += SYSTEM_PAGE_SIZE;
        paddr += SYSTEM_PAGE_SIZE;
    }
}

void initialize_modules() {

/*
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(platform.modules, &iter);
    info_module_t* module;
    while ((module = linkedlist_iter_next(&iter))) {
        log_info("initializing module '%s' at %p (size %d bytes)", module->param, module->region.addr_start, module->region.size);

        elf_t elf;
        if (elf_parser((void*) module->region.addr_start, module->region.size, &elf) != ELF_SUCCESS) {
            log_warn("Invalid module: %s", module->param);
            continue;
        }
        log_info("Entry point=%p", elf.entry_point);

        native_page_table_t* pt = hal_page_table_create_mapping();
        fill_kernel_pages(pt);

        linkedlist_iter_t phiter;
        linkedlist_iter_initialize(elf.program_headers, &phiter);
        elf_program_header_t* program_header;
        while ((program_header = (elf_program_header_t*) linkedlist_iter_next(&phiter))) {
            if (program_header->type == ELF_PT_LOAD) {
                allocate_program_header(program_header, pt);
            }
        }
        char name[] = "hello";
        uintptr_t stack_addr = page_allocator_allocate();
        hal_page_table_add_mapping(pt, stack_addr, stack_addr, true, false, true);
        task_id_t task = task_create(name, pt);
        log_info("task=%d", task);
        task_start(task, elf.entry_point, stack_addr + SYSTEM_PAGE_SIZE);
    }

*/
    native_page_table_t* pt = hal_page_table_create_mapping();
    fill_kernel_pages(pt);

    uintptr_t stack_addr = (uintptr_t) kmem_alloc(8192);
    stack_addr += 4000;
    task_id_t task1 = task_create("user1", pt);
    task_start(task1, (uintptr_t) &user_task1, stack_addr);

    // task_id_t task2 = task_create("user2", pt);
    // task_start(task2, (uintptr_t) &user_task2);

    // task_id_t task3 = task_create("user3", pt);
    // task_start(task3, (uintptr_t) &user_task3);
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
    linkedlist_iter_initialize(platform.memory.memory_segments, &iter);
    multiboot_memory_map_t* mmap;
    while ((mmap = (multiboot_memory_map_t*) linkedlist_iter_next(&iter))) {
        page_allocator_mark_as_system(mmap->addr, mmap->len);
    }

    // mark pages of the kernel
    page_allocator_mark_as_system(platform.memory.kernel.addr_start, platform.memory.kernel.size);

    task_management_initialize();
    
    log_info("System ready");

    initialize_modules();
}
