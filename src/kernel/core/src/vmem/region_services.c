#include <logging.h>
#include <core/alloc.h>
#include <core/hal/platform.h>
#include <core/hal/native_vmem.h>
#include <core/vmem/services.h>
#include <core/pmem/services.h>

static id_mapper_t vmem_region_mapper;

bool vmem_region_initialize()
{
    if (!id_mapper_create(&vmem_region_mapper)) {
        log_fatal("Error creating id mapper for virtual memory regions");
        return false;
    }
    return true;
}

/**
 * Appends a new physical page of memory to the region.
 * Returns 0 in case of failure or the virtual address.
 * 
 * This function doesn't flush the page table and doesn't increase the size (only allocated_size)
 */
static uintptr_t vmem_region_assoc_physical_address(vmem_region_t* vmem_region, uintptr_t paddr)
{
    uintptr_t vaddr = vmem_region->start + vmem_region->allocated_size;

    // first step... increase the size of the region
    // It is essential keep the increase of the region size before append in the list
    // to avoid a infinite loop in the kalloc (resizing the kernel data area + linkedlist_append)
    vmem_region->allocated_size += SYSTEM_PAGE_SIZE;

    // stores the page utilized in case of release
    linkedlist_append(vmem_region->pages, (void*) paddr);

    WHILE_LINKEDLIST_ITER(vmem_region->attached, vmem_t*, vmem) {
        if (!vmem_notify_change(vmem, vmem_region, paddr, vaddr)) {
            return 0;
        }
    }
    return vaddr;
}

static void flush_if_required()
{
    // TODO flush only if required
    native_vmem_flush();
}

uintptr_t vmem_region_map_physical_address(vmem_region_t* vmem_region, uintptr_t physical_start_addr, size_t size)
{
    if (!vmem_region) {
        log_warn("Invalid memory region: %p", vmem_region);
        return 0;
    }

    if (physical_start_addr != MEM_ALIGN(physical_start_addr) || size != MEM_ALIGN(size)) {
        log_warn("Mapping address to region %s is unaligned: From %p and size %d", vmem_region->name, physical_start_addr, size);
        return 0;
    }

    uintptr_t paddr = physical_start_addr;
    size_t remaining = size;
    uintptr_t returning_addr = 0;
    while (remaining > 0) {
        uintptr_t vaddr = vmem_region_assoc_physical_address(vmem_region, paddr);
        if (!vaddr) {
            log_error("Failure associating physical address %p to region %s", paddr, vmem_region->name);
            return 0;
        }

        // the first allocation is returned
        if (paddr == physical_start_addr) {
            returning_addr = vaddr;
        }

        paddr += SYSTEM_PAGE_SIZE;
        remaining -= SYSTEM_PAGE_SIZE;

        // in this case size grows until the allocated area
        vmem_region->size = vmem_region->allocated_size;
    }

    log_trace("Mapped on region %s virtual address %p-%p to physical %p-%p", 
        vmem_region->name, returning_addr, returning_addr + size, physical_start_addr, physical_start_addr + size);
    flush_if_required();
    // vmem_region_dump(region);
    return returning_addr;
}

// TODO Change this method to a iteration in all memory region of a vmem
static uintptr_t find_next_memory_address(vmem_t* vmem)
{
    uintptr_t addr = vmem->next_start_address;
    vmem->next_start_address += REGION_ADDRESS_INCREMENT;
    return addr;

    /*
    // old code to find the next memory address available 
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(memory->map, &iter);
    memory_map_t* next;
    while ((next = linkedlist_iter_next(&iter))) {
        addr = MAX(addr, next->virtual_addr + next->region->size);
    }
    // align memory address
    uintptr_t aligned_addr = MEM_ALIGN(addr);
    if (aligned_addr != addr) {
        aligned_addr += SYSTEM_PAGE_SIZE;
    }
    return aligned_addr;
    */
}

vmem_region_t* vmem_region_create(vmem_t* vmem, const char* name, uintptr_t start, size_t size, bool user, bool writable, bool executable)
{
    if (!vmem || !name) {
        log_warn("Invalid memory reference to create the region %s", name);
        return NULL;
    }

    if (start == 0) {
        // next memory address available
        start = find_next_memory_address(vmem);
    }

    if (MEM_ALIGN(start) != start) {
        log_warn("Invalid memory region %s, starting with an unaligned address: %p", name, start);
        return NULL;
    }

    // TODO There is any memory address constraint?

    vmem_region_t* vmem_region = NEW(vmem_region_t);
    if (!vmem_region) {
        log_error("Error allocation memory to a new virtual memory region");
        return NULL;
    }
    vmem_region->name = name;
    vmem_region->start = start;
    vmem_region->executable = executable;
    vmem_region->writable = writable;
    vmem_region->size = 0;  // allocates with size 0 and expand after.
    vmem_region->allocated_size = 0;
    vmem_region->user = user;
    vmem_region->pages = linkedlist_create();

    vmem_region->attached = linkedlist_create();
    if (!vmem_attach(vmem, vmem_region)) {
        log_error("Error attaching new region %s to memory %p", vmem_region->name, vmem);
        goto error;
    }

    if (!vmem_region_resize(vmem_region, size)) {
        goto error;
    }

    vmem_region->vmem_region_id = id_mapper_add(&vmem_region_mapper, vmem_region);
    if (!vmem_region->vmem_region_id) {
        goto error;
    }
    log_trace("Created region %s of size 0x%x (%d KB) starting (virtual) at %p", name, size, size / 1024, start);
    return vmem_region;

error:
    // clean up resources
    linkedlist_destroy(vmem_region->attached);
    linkedlist_destroy(vmem_region->pages);
    FREE(vmem_region);
    log_error("Error creating a new virtual memory region");
    return NULL;
}

bool vmem_region_resize(vmem_region_t* vmem_region, size_t new_size)
{
    if (!vmem_region) {
        return false;
    }

    bool failure = false;
    size_t new_allocated_size = ALIGN_NEXT(new_size, SYSTEM_PAGE_SIZE);
    if (new_allocated_size < vmem_region->allocated_size) {
        // TODO release pages not implemented yet
        log_warn("Shinking memory region %s not complete implemented", vmem_region->name);
        vmem_region->size = new_size;

    } else {
        // alocate all necessary pages
        size_t remain_space = new_allocated_size - vmem_region->allocated_size;
        while (remain_space > 0) {
            uintptr_t paddr = pmem_allocate();
            if (!paddr) {
                log_warn("No more memory for region %s", vmem_region->name);
                failure = true;
                break;
            }

            vmem_region_assoc_physical_address(vmem_region, paddr);
            remain_space -= SYSTEM_PAGE_SIZE;
        }

        // adjust the size only if there is enough allocated_size for it (in case of no more memory)
    }

    if (new_size <= vmem_region->allocated_size) {
        vmem_region->size = new_size;
    }
    flush_if_required();
    // vmem_region_dump(region);
    return !failure;
}

void vmem_region_dump(vmem_region_t* vmem_region)
{
    if (!vmem_region) {
        log_warn("vmem_region_dump called with NULL region");
        return;
    }

    log_trace("=========== Dump memory region ==============");
    log_trace("Name: %s \t Metadata: %p \t Attached: %d",
        vmem_region->name,
        vmem_region,
        linkedlist_size(vmem_region->attached));
    log_trace("Start Address: %p \t End (alloc): %p \t Size: %d (%d KB) bytes \t Allocated: %d KB", 
        vmem_region->start,
        vmem_region->start + vmem_region->allocated_size - 1,
        vmem_region->size,
        vmem_region->size / 1024,
        vmem_region->allocated_size / 1024);
    log_trace("Permissions: %c%c%c%c \t Physical pages: %d \t First Page Address: %p",
        vmem_region->user?'u':'-', 'r',
        vmem_region->writable?'w':'-',
        vmem_region->executable?'x':'-',
        linkedlist_size(vmem_region->pages),
        linkedlist_get(vmem_region->pages, 0)
        );
    log_trace("---------------------------------------------");
    
    /*
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(region->pages, &iter);
    uintptr_t page_addr = 0;
    while ((page_addr = (uintptr_t) linkedlist_iter_next(&iter))) {
        log_trace("Page at %p", page_addr);
    }*/
}
