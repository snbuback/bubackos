#include <logging.h>
#include <libutils/id_mapper.h>
#include <libutils/utils.h>
#include <core/hal/platform.h>
#include <core/alloc.h>
#include <core/vmem/services.h>
#include <core/hal/native_vmem.h>

static id_mapper_t vmem_id_mapper;
static vmem_t* kernel_vmem;

bool vmem_initialize() {
    if (!id_mapper_create(&vmem_id_mapper)) {
        log_fatal("Failure initializing id mapper for module vmem");
        return false;
    }

    // initialize kernel virtual memory
    kernel_vmem = vmem_create();
    return true;
}

vmem_t* vmem_get_kernel()
{
    return kernel_vmem;
}

vmem_t* vmem_create()
{
    vmem_t* vmem = NEW(vmem_t);
    if (!vmem) {
        log_error("Error allocating memory to a new virtual memory object");
        return NULL;
    }
    vmem->next_start_address = REGION_ADDRESS_INCREMENT;
    vmem->regions = linkedlist_create();
    vmem->map = linkedlist_create();
    if (!native_vmem_create(vmem)) {
        goto error;
    }

    // kernel pages are always attached to any virtual memory.
    if (kernel_vmem) {
        WHILE_LINKEDLIST_ITER(kernel_vmem->regions, vmem_region_t*, vmem_region) {
            if (!vmem_attach(vmem, vmem_region)) {
                goto error;
            }
        }
    }

    vmem->vmem_id = id_mapper_add(&vmem_id_mapper, vmem);
    if (!vmem->vmem_id) {
        goto error;
    }
    return vmem;

error:
    log_error("Failure create new virtual memory object");
    vmem_destroy(vmem);
    return NULL;
}

void vmem_destroy(vmem_t* vmem)
{
    if (!vmem) {
        return;
    }
    linkedlist_destroy(vmem->regions);
    linkedlist_destroy(vmem->map);

    // TODO unmap the memory region
    FREE(vmem);
}

vmem_t* vmem_get(vmem_id_t vmem_id)
{
    return id_mapper_get(&vmem_id_mapper, vmem_id);
}

/**
 * Notify a memory handler that a region of memory was updated
 */
bool vmem_notify_change(vmem_t* vmem, vmem_region_t* region, uintptr_t paddr, uintptr_t vaddr)
{
    vmem_map_t* map = NEW(vmem_map_t);
    if (!map) {
        return false;
    }
    map->physical_addr = paddr;
    map->virtual_addr = vaddr;
    map->region = region;

    if (!linkedlist_append(vmem->map, map)) {
        return false;
    }

    // map the page in the tlb
    page_map_entry_t entry = {
        .virtual_addr = vaddr,
        .physical_addr = paddr,
        .size = SYSTEM_PAGE_SIZE,
        .permission = 0,
        .present = true,
    };

    PERM_SET_KERNEL_MODE(entry.permission, !map->region->user);
    PERM_SET_WRITE(entry.permission, map->region->writable);
    PERM_SET_EXEC(entry.permission, map->region->executable);
    PERM_SET_READ(entry.permission, true);
    native_vmem_set(vmem, entry);
    return true;
}

/**
 * Returns the physical address of a given virtual address
 */
uintptr_t vmem_get_physical_address(vmem_t* vmem, uintptr_t vaddr)
{
    // align address and calculate the shifting
    uintptr_t aligned = MEM_ALIGN(vaddr);
    uintptr_t shifting = vaddr - aligned;

    WHILE_LINKEDLIST_ITER(vmem->map, vmem_map_t*, map) {
        if (map->virtual_addr == aligned) {
            return map->physical_addr + shifting;
        }
    }
    log_warn("Invalid get of physical memory address: %p", vaddr);
    return 0;
}

bool vmem_attach(vmem_t* vmem, vmem_region_t* vmem_region)
{
    if (!vmem || !vmem_region) {
        log_error("Attaching invalid memory or region references. Mem=%p region=%p", vmem, vmem_region);
        return false;
    }

    // verify if the region already exist
    if (linkedlist_find(vmem->regions, vmem_region) != -1) {
        log_error("Region %s is already attached on memory %p", vmem_region->name, vmem);
        return true;
    }

    if (linkedlist_append(vmem->regions, vmem_region)) {
        if (linkedlist_append(vmem_region->attached, vmem)) {

            // update the memory with all existed pages
            uintptr_t vaddr = vmem_region->start;
            WHILE_LINKEDLIST_ITER(vmem_region->pages, uintptr_t, paddr) {
                if (!vmem_notify_change(vmem, vmem_region, paddr, vaddr)) {
                    goto error;
                }
                vaddr += SYSTEM_PAGE_SIZE;
            }
            return true;
        }
    }

error:
    // rollback all changes.
    // Unfortunatelly vmem_notify_change could left the vmem in a invalid state.
    linkedlist_remove_element(vmem->regions, vmem_region);
    linkedlist_remove_element(vmem_region->attached, vmem);
    log_error("Error attaching region %s on memory %p. Possible inconsistent state!", vmem_region->name, vmem);
    return false;
}

void vmem_dump(vmem_t* vmem)
{
    if (!vmem) {
        return;
    }

    log_trace("====================== Dump memory =========================");
    log_trace("Memory addr: %p", vmem);
    log_trace("Num. regions: %d \t num. of maps: %d \t Next start address: %p",
        linkedlist_size(vmem->regions),
        linkedlist_size(vmem->map),
        vmem->next_start_address);

    WHILE_LINKEDLIST_ITER(vmem->regions, vmem_region_t*, region) {
        vmem_region_dump(region);
    }
    log_trace("------------------------------------------------------------");
}

