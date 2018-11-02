#include <stdbool.h>
#include <string.h>
#include <logging.h>
#include <algorithms/linkedlist.h>
#include <libutils/utils.h>
#include <core/hal/platform.h>
#include <core/loader/module_loader.h>
#include <core/vmem/services.h>
#include <core/task/services.h>
#include <core/loader/elf.h>

// TODO change calls to memory management thru syscall so this is no more required.
vmem_t* memory_handler;

bool allocate_program_header(elf_t* elf, elf_program_header_t* ph, vmem_t* module_memory_handler)
{
    bool writable = ph->flags & ELF_PF_FLAGS_W;
    bool code = true; //ph->flags & ELF_PF_FLAGS_X;
    vmem_region_t* region = vmem_region_create(module_memory_handler, "elf-loader", ph->vaddr, 0, true, writable, code);
    if (!region) {
        return false;
    }
    // map the content of file to the allocated region
    if (ph->file_size) {
        if (!vmem_region_map_physical_address(region, vmem_get_physical_address(memory_handler, elf->base + ph->offset), ALIGN_NEXT(ph->file_size, SYSTEM_PAGE_SIZE))) {
            return false;
        }
    }

    // resize region to fits mem_size
    if (!vmem_region_resize(region, ph->mem_size)) {
        return false;
    }
    // TODO missing fill content with zeros.
    return true;
}

bool module_task_initialize()
{
    log_info("Initializing modules");
    platform_t* platform = get_platform_config();

    WHILE_LINKEDLIST_ITER(platform->modules, info_module_t*, module) {
        const char* module_name = module->param;
        log_info("initializing module '%s' at %p (physical) (size %d bytes)", module_name, module->region.addr_start, module->region.size);

        vmem_region_t* region = vmem_region_create(memory_handler, "module-loading", 0, 0, true, true, true);
        uintptr_t vaddr = vmem_region_map_physical_address(region, module->region.addr_start, ALIGN_NEXT(module->region.size, SYSTEM_PAGE_SIZE));
        if (!vaddr) {
            return false;
        }

        elf_t elf;
        if (elf_parser((void*) vaddr, module->region.size, &elf) != ELF_SUCCESS) {
            log_warn("Invalid module: %s", module_name);
            continue;
        }
        log_trace("Module entry point=%p", elf.entry_point);

        vmem_t* module_memory_handler = vmem_create();

        bool abort = false;
        WHILE_LINKEDLIST_ITER(elf.program_headers, elf_program_header_t*, program_header) {
            if (program_header->type == ELF_PT_LOAD) {
                if (!allocate_program_header(&elf, program_header, module_memory_handler)) {
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

        task_t* task = task_create(module_name, module_memory_handler);
        task_set_kernel_mode(task);
        // const char* args[] = {"primeiro", "segundo", "terceiro!!!"};
        // task_set_arguments(task, 3, args);
        task_run(task, elf.entry_point);
    }
    return true;
}

bool module_initialize()
{
    memory_handler = vmem_get_kernel();
    module_task_initialize();
    return true;
    // TODO TO revert the use of kernel pages is necessary allow task_set_arguments runs with the same handler as this task
    // memory_handler = vmem_create();
    // task_id_t task = task_create("module_init", memory_handler);
    // task_set_kernel_mode(task);
    // task_start(task, (uintptr_t) &mod_init);
    // return true;
}

