#include <core/init.h>
#include <logging.h>
#include <core/page_allocator.h>
#include <core/task_management.h>
#include <hal/configuration.h>
#include <core/elf.h>
#include <core/memory.h>
#include <string.h>

extern int user_task1;
extern int user_task2;
extern int user_task3;

// TODO change platform to an argument. After the kernel initialize doesn't make sense keep this value (check how to implement memory allocation first)
extern platform_t platform;

static bool copy_program_header(uintptr_t virtual_address, void* src, size_t size, memory_t* memory_handler)
{
    while (size > 0) {
        void* paddr = (void*) memory_management_get_physical_address(memory_handler, virtual_address);
        if (!paddr) {
            log_warn("No physical address associate with the virtual address %p", virtual_address);
            return false;
        }
        size_t block_to_copy = MIN((size_t) SYSTEM_PAGE_SIZE, size);
        log_trace("Copying %d bytes from %p to %p (virtual=%p)", block_to_copy, src, paddr, virtual_address);
        memcpy(paddr, src, block_to_copy);
        size -= block_to_copy;
        virtual_address += block_to_copy;
        src += block_to_copy;
    }
    return true;
}

bool allocate_program_header(elf_t* elf, elf_program_header_t* ph, memory_t* memory_handler)
{
    // bool writable = ph->flags & ELF_PF_FLAGS_W;
    // bool code = ph->flags & ELF_PF_FLAGS_X;
    memory_region_t* region = memory_management_region_create(memory_handler, ph->vaddr, ph->mem_size, true, true, true);
    if (!region) {
        return false;
    }
    // copy content from file to the allocated region
    if (!copy_program_header(ph->vaddr, (void*) (elf->base + ph->offset), ph->file_size, memory_handler)) {
        return false;
    }

    return true;
    // TODO missing fill content with zeros.
}

void initialize_modules() {

    log_info("Initializing modules...");

    linkedlist_iter_t iter;
    linkedlist_iter_initialize(platform.modules, &iter);
    info_module_t* module;
    while ((module = linkedlist_iter_next(&iter))) {
        const char* module_name = module->param;
        log_info("initializing module '%s' at %p (size %d bytes)", module_name, module->region.addr_start, module->region.size);

        elf_t elf;
        if (elf_parser((void*) module->region.addr_start, module->region.size, &elf) != ELF_SUCCESS) {
            log_warn("Invalid module: %s", module_name);
            continue;
        }
        log_trace("Module entry point=%p", elf.entry_point);

        memory_t* memory_handler = memory_management_create();

        bool abort = false;
        linkedlist_iter_t phiter;
        linkedlist_iter_initialize(elf.program_headers, &phiter);
        elf_program_header_t* program_header;
        while ((program_header = (elf_program_header_t*) linkedlist_iter_next(&phiter))) {
            if (program_header->type == ELF_PT_LOAD) {
                if (!allocate_program_header(&elf, program_header, memory_handler)) {
                    abort = true;
                    break;
                }
            }
        }

        if (!abort) {
            task_id_t task = task_create(module_name, memory_handler);
            task_set_kernel_mode(task);
            const char* args[] = {"primeiro", "segundo", "terceiro!!!"};
            task_set_arguments(task, 3, args);
            task_start(task, elf.entry_point);
        } else {
            // TODO Release memory
            log_warn("Module %s aborted", module_name);
        }

    }
}

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

    log_info("System ready");

    initialize_modules();
}
