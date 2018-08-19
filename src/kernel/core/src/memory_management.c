#include <hal/configuration.h>
#include <core/memory_management.h>
#include <core/memory.h>
#include <libutils/utils.h>
#include <core/page_allocator.h>
#include <logging.h>

static volatile memory_id_t last_id;
static linkedlist_t* list_of_memory; // list of memory_t

void memory_management_initialize() {
    list_of_memory = linkedlist_create();
    last_id = 0;
    // TODO Kernel memory??? (maybe it is not responsibility of this module)
}

memory_t* memory_management_create()
{
    memory_t* memory = NEW(memory_t);
    memory->id = ++last_id;
    memory->pt = native_pagetable_create();
    memory->regions = linkedlist_create();
    memory->map = linkedlist_create();

    native_page_table_t* pt = memory->pt;
    for (uintptr_t addr = 0; addr <= 8*1024*1024; addr += SYSTEM_PAGE_SIZE) {
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

static void fill_region_pages(memory_region_t* region)
{
    // alocate all necessary pages
    size_t remain_space = region->size;
    uintptr_t vaddr = region->start;
    while (remain_space > 0) {
        uintptr_t paddr = page_allocator_allocate();
        if (!paddr) {
            log_warn("No more memory");
            return;
        }
        // TODO no page release so far, so I don't need to save
        // linkedlist_append(paddr);
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
        memory_map_t* map = NEW(memory_map_t);
        map->physical_addr = paddr;
        map->virtual_addr = vaddr;
        map->region = region;
        linkedlist_append(region->memory->map, map);
        
        remain_space -= MIN(remain_space, (size_t) SYSTEM_PAGE_SIZE);  // size_t is unsigned
        vaddr += SYSTEM_PAGE_SIZE;
    }
}

memory_region_t* memory_management_region_create(memory_t* memory, uintptr_t start, size_t size, bool user, bool writable, bool executable)
{
    if (!memory) {
        return NULL;
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
    region->start = MEM_ALIGN(start);
    region->executable = executable;
    region->writable = writable;
    region->size = size;
    region->user = user;
    region->pages = linkedlist_create();

    fill_region_pages(region);
    log_trace("Created region %p of size %x starting (virtual) at %p", region, size, start);
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
