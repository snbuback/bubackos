#include <hal/configuration.h>
#include <core/memory_management.h>
#include <core/memory.h>
#include <libutils/utils.h>
#include <core/page_allocator.h>
#include <logging.h>

#define REGION_ADDRESS_INCREMENT        (1*1024*1024)   // 1GB

static volatile memory_id_t last_id;
static linkedlist_t* list_of_memory; // list of memory_t
static memory_t* kernel_mem;

void memory_management_initialize() {
    list_of_memory = linkedlist_create();
    last_id = 0;
    kernel_mem = memory_management_create();
    // TODO Kernel memory??? (maybe it is not responsibility of this module)
}

memory_t* memory_management_get_kernel()
{
    return kernel_mem;
}

memory_t* memory_management_create()
{
    memory_t* memory = NEW(memory_t);
    memory->id = ++last_id;
    memory->next_start_address = REGION_ADDRESS_INCREMENT;
    memory->pt = native_pagetable_create();
    memory->regions = linkedlist_create();
    memory->map = linkedlist_create();

    native_page_table_t* pt = memory->pt;
    for (uintptr_t addr = 0; addr <= 2*1024*1024; addr += SYSTEM_PAGE_SIZE) {
        page_map_entry_t entry = {
            .virtual_addr = addr,
            .physical_addr = addr,
            .size = SYSTEM_PAGE_SIZE,
            .permission = 0,
            .present = true,
            };
        PERM_SET_KERNEL_MODE(entry.permission, true);
        PERM_SET_WRITE(entry.permission, true);
        PERM_SET_READ(entry.permission, true);
        PERM_SET_EXEC(entry.permission, true);
        native_pagetable_set(pt, entry);
    }

    // add to the global list
    linkedlist_append(list_of_memory, memory);
    return memory;
}

/**
 * Appends a new physical page of memory to the region.
 * Returns 0 in case of failure or the virtual address.
 * 
 * This function doesn't flush the page table.
 */
static uintptr_t region_assoc_physical_address(memory_region_t* region, uintptr_t paddr)
{
    uintptr_t vaddr = region->start + region->size;

    // first step... increase the size of the region
    // It is essential keep the increase of the region size before append in the list
    // to avoid a infinite loop in the malloc (resizing the kernel data area + linkedlist_append)
    region->size += SYSTEM_PAGE_SIZE;

    // stores the page utilized in case of release
    linkedlist_append(region->pages, (void*) paddr);
    memory_map_t* map = NEW(memory_map_t);
    map->physical_addr = paddr;
    map->virtual_addr = vaddr;
    map->region = region;
    linkedlist_append(region->memory->map, map);

    // map the page in the tlb
    page_map_entry_t entry = {
        .virtual_addr = vaddr,
        .physical_addr = paddr,
        .size = SYSTEM_PAGE_SIZE,
        .permission = 0,
        .present = true,
    };

    PERM_SET_KERNEL_MODE(entry.permission, !region->user);
    PERM_SET_WRITE(entry.permission, region->writable);
    PERM_SET_EXEC(entry.permission, region->executable);
    PERM_SET_READ(entry.permission, true);
    native_pagetable_set(region->memory->pt, entry);
    return vaddr;
}

static void flush_if_required()
{
    // TODO flush only if required
    native_page_table_flush();
}

uintptr_t memory_management_map_physical_address(memory_region_t* region, uintptr_t paddr)
{
    uintptr_t vaddr = region_assoc_physical_address(region, paddr);
    if (vaddr) {
        log_debug("## Associating vaddr=%p with paddr=%p on region %p (%d KB of size)", vaddr, paddr, region, region->size/1024);
        flush_if_required();
    }
    return vaddr;
}

static uintptr_t find_next_memory_address(memory_t* memory)
{
    uintptr_t addr = memory->next_start_address;
    memory->next_start_address += REGION_ADDRESS_INCREMENT;
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

memory_region_t* memory_management_region_create(memory_t* memory, uintptr_t start, size_t size, bool user, bool writable, bool executable)
{
    if (!memory) {
        return NULL;
    }

    if (start == 0) {
        // next memory address available
        start = find_next_memory_address(memory);
    }

    if (MEM_ALIGN(start) != start) {
        log_warn("Invalid memory region, starting with an unaligned address: %p", start);
        return NULL;
    }

    // TODO There is any memory address constraint?
    // if (start < ???) {
    //  return NULL
    // }

    memory_region_t* region = NEW(memory_region_t);
    region->memory = memory;
    region->start = start;
    region->executable = executable;
    region->writable = writable;
    region->size = 0;  // allocates with size 0 and expand after.
    region->user = user;
    region->pages = linkedlist_create();

    memory_management_region_resize(region, size);
    log_trace("Created region %p of size 0x%x (%d KB) starting (virtual) at %p", region, size, size / 1024, start);
    return region;
}

/**
 * Returns the physical address of a given virtual address
 */
uintptr_t memory_management_get_physical_address(memory_t* mhandler, uintptr_t vaddr)
{
    // align address and calculate the shifting
    uintptr_t aligned = MEM_ALIGN(vaddr);
    uintptr_t shifting = vaddr - aligned;

    linkedlist_iter_t iter;
    linkedlist_iter_initialize(mhandler->map, &iter);
    memory_map_t* map;
    while ((map = linkedlist_iter_next(&iter))) {
        if (map->virtual_addr == aligned) {
            return map->physical_addr + shifting;
        }
    }
    return 0;
}

bool memory_management_region_resize(memory_region_t* region, size_t new_size)
{
    if (!region) {
        return false;
    }

    new_size = ALIGN_NEXT(new_size, SYSTEM_PAGE_SIZE);
    if (region->size == new_size) {
        // nothing to do
        return true;
    }

    if (region->size > new_size) {
        // TODO shinking not implemented yet
        log_warn("Shinking memory region not implemented");
    } else {
        // alocate all necessary pages
        size_t remain_space = new_size - region->size;
        while (remain_space > 0) {
            uintptr_t paddr = page_allocator_allocate();
            if (!paddr) {
                log_warn("No more memory");
                flush_if_required();
                return false;
            }

            region_assoc_physical_address(region, paddr);
            remain_space -= SYSTEM_PAGE_SIZE;
        }
    }

    flush_if_required();
    return true;
}
