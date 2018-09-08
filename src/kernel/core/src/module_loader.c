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

// TODO change calls to memory management thru syscall so this is no more required.
memory_t* memory_handler;

bool allocate_program_header(elf_t* elf, elf_program_header_t* ph, memory_t* memory_handler)
{
    // bool writable = ph->flags & ELF_PF_FLAGS_W;
    // bool code = ph->flags & ELF_PF_FLAGS_X;
    memory_region_t* region = memory_management_region_create(memory_handler, "?-loader", ph->vaddr, 0, true, true, true);
    if (!region) {
        return false;
    }
    // map the content of file to the allocated region
    if (ph->file_size) {
        if (!memory_management_region_map_physical_address(region, elf->base + ph->offset, ALIGN_NEXT(ph->file_size, SYSTEM_PAGE_SIZE))) {
            return false;
        }
    }

    // resize region to fits mem_size
    if (!memory_management_region_resize(region, ph->mem_size)) {
        return false;
    }
    // TODO missing fill content with zeros.
    return true;
}

bool module_task_initialize()
{
    log_info("Initializing modules...\n\n\n\n");

    WHILE_LINKEDLIST_ITER(platform.modules, info_module_t*, module) {
        const char* module_name = module->param;
        log_info("initializing module '%s' at %p (size %d bytes)", module_name, module->region.addr_start, module->region.size);

        memory_region_t* region = memory_management_region_create(memory_handler, "module-loading", 0, 0, true, true, true);
        uintptr_t vaddr = memory_management_region_map_physical_address(region, module->region.addr_start, ALIGN_NEXT(module->region.size, SYSTEM_PAGE_SIZE));
        if (!vaddr) {
            return false;
        }

        elf_t elf;
        if (elf_parser((void*) vaddr, module->region.size, &elf) != ELF_SUCCESS) {
            log_warn("Invalid module: %s", module_name);
            continue;
        }
        log_trace("Module entry point=%p", elf.entry_point);

        memory_t* memory_handler = memory_management_create();

        bool abort = false;
        WHILE_LINKEDLIST_ITER(elf.program_headers, elf_program_header_t*, program_header) {
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
        // task_set_kernel_mode(task);
        // const char* args[] = {"primeiro", "segundo", "terceiro!!!"};
        // task_set_arguments(task, 3, args);
        task_start(task, elf.entry_point);
    }
    return true;

}

static void mod_init()
{
    module_task_initialize();
        asm ("xchg %bx, %bx");

    asm volatile ("movq $1, %rdi; int $50");
    for(;;);
}

bool module_initialize()
{
    memory_handler = memory_management_create();
    task_id_t task = task_create("module_init", memory_handler);
    task_set_kernel_mode(task);
    task_start(task, (uintptr_t) &mod_init);
    return true;
}

