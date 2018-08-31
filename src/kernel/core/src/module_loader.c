#include <stdbool.h>
#include <string.h>
#include <algorithms/linkedlist.h>
#include <core/module_loader.h>
#include <core/memory_management.h>
#include <core/task_management.h>
#include <core/elf.h>
#include <libutils/utils.h>
#include <hal/configuration.h>
#include <logging.h>
#include <hal/platform.h>

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
    memory_region_t* region = memory_management_region_create(memory_handler, "?-loader", ph->vaddr, ph->mem_size, true, true, true);
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

bool module_task_initialize()
{
    log_info("Initializing modules...\n\n\n\n");

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

        if (abort) {
            // TODO Release memory
            log_warn("Module %s aborted", module_name);
            return false;
        }

        task_id_t task = task_create(module_name, memory_handler);
        task_set_kernel_mode(task);
        const char* args[] = {"primeiro", "segundo", "terceiro!!!"};
        task_set_arguments(task, 3, args);
        task_start(task, elf.entry_point);
    }
    return true;
}

bool module_initialize()
{
    task_id_t task_id = task_create("module_initialize", memory_management_get_kernel());
    if (!task_id) {
        return false;
    }
    if (!task_set_kernel_mode(task_id)) {
        return false;
    }
    if (!task_start(task_id, (uintptr_t) &module_task_initialize)) {
        return false;
    }
    return true;
}

