#include <hal/configuration.h>
#include <core/memory_management.h>
#include <core/memory.h>
#include <libutils/utils.h>
#include <core/page_allocator.h>
#include <logging.h>
#include <hal/platform.h>
#include <stdbool.h>

#define REGION_ADDRESS_INCREMENT        (1*1024*1024)   // 1GB

static volatile memory_id_t last_id;
static linkedlist_t* list_of_memory; // list of memory_t
static memory_t* kernel_mem;
memory_region_t* kernel_code_region;
memory_region_t* kernel_data_region;

static bool map_current_kernel_memory_address(memory_t* mem)
{
    // code
    kernel_code_region = memory_management_region_create(
        mem,
        "kernel-code",
        platform.memory.kernel.addr_start,
        0,
        false,
        true,
        true
    );

    if (!memory_management_region_map_physical_address(kernel_code_region, platform.memory.kernel.addr_start, ALIGN_NEXT(platform.memory.kernel.size, SYSTEM_PAGE_SIZE))) {
        return false;
    }

    // data
    // TODO Since hal_switch_task uses pushq it is required execution permission on kernel memory
    uintptr_t data_addr = platform.memory.kernel_data.addr_start;
    kernel_data_region = memory_management_region_create(
        kernel_mem,
        "kernel-data",
        data_addr,
        0,
        false,
        true,
        true
    );

    // for data, always allocates more 1 page to ensure any allocation before the memory allocator module
    // initialise still fits in the region mapped.
    size_t new_size = ALIGN_NEXT(platform.memory.kernel_data.size + 100*SYSTEM_PAGE_SIZE, SYSTEM_PAGE_SIZE);
    if (!memory_management_region_map_physical_address(
        kernel_data_region, platform.memory.kernel_data.addr_start,
        new_size)
    ) {
        platform.memory.kernel_data.size = new_size;
        return false;
    }
    return true;
}

bool memory_management_initialize() {
    list_of_memory = linkedlist_create();
    if (!list_of_memory) {
        return false;
    }
    last_id = 0;
    kernel_mem = memory_management_create();
    if (!map_current_kernel_memory_address(kernel_mem)) {
        return false;
    }

    log_info("Memory management initialised. Switching to kernel pages.");
    native_pagetable_switch(kernel_mem->pt);
    return true;
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

    // TODO While I don't have memory_region inheritance I need to ensure kernel page are allocated in each module
    if (kernel_code_region && kernel_data_region) {
        memory_management_attach(memory, kernel_code_region);
        memory_management_attach(memory, kernel_data_region);
    }

    // add to the global list
    linkedlist_append(list_of_memory, memory);
    return memory;
}

/**
 * Notify a memory handler that a region of memory was updated
 */
static bool add_new_memory_map(memory_t* memory, memory_region_t* region, uintptr_t paddr, uintptr_t vaddr)
{
    memory_map_t* map = NEW(memory_map_t);
    if (!map) {
        return false;
    }
    map->physical_addr = paddr;
    map->virtual_addr = vaddr;
    map->region = region;

    if (!linkedlist_append(memory->map, map)) {
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
    native_pagetable_set(memory->pt, entry);
    return true;
}

/**
 * Appends a new physical page of memory to the region.
 * Returns 0 in case of failure or the virtual address.
 * 
 * This function doesn't flush the page table and doesn't increase the size (only allocated_size)
 */
static uintptr_t region_assoc_physical_address(memory_region_t* region, uintptr_t paddr)
{
    uintptr_t vaddr = region->start + region->allocated_size;

    // first step... increase the size of the region
    // It is essential keep the increase of the region size before append in the list
    // to avoid a infinite loop in the kalloc (resizing the kernel data area + linkedlist_append)
    region->allocated_size += SYSTEM_PAGE_SIZE;

    // stores the page utilized in case of release
    linkedlist_append(region->pages, (void*) paddr);

    WHILE_LINKEDLIST_ITER(region->attached, memory_t*, memory) {
        if (!add_new_memory_map(memory, region, paddr, vaddr)) {
            return 0;
        }
    }
    return vaddr;
}

static void flush_if_required()
{
    // TODO flush only if required
    native_page_table_flush();
}

uintptr_t memory_management_region_map_physical_address(memory_region_t* region, uintptr_t physical_start_addr, size_t size)
{
    if (!region) {
        log_warn("Invalid memory region: %p", region);
        return 0;
    }

    if (physical_start_addr != MEM_ALIGN(physical_start_addr) || size != MEM_ALIGN(size)) {
        log_warn("Mapping address to region %s is unaligned: From %p and size %d", region->region_name, physical_start_addr, size);
        return 0;
    }

    uintptr_t paddr = physical_start_addr;
    size_t remaining = size;
    uintptr_t returning_addr = 0;
    while (remaining > 0) {
        uintptr_t vaddr = region_assoc_physical_address(region, paddr);
        if (!vaddr) {
            log_error("Failure associating physical address %p to region %s", paddr, region->region_name);
            return 0;
        }

        // the first allocation is returned
        if (paddr == physical_start_addr) {
            returning_addr = vaddr;
        }

        paddr += SYSTEM_PAGE_SIZE;
        remaining -= SYSTEM_PAGE_SIZE;

        // in this case size grows until the allocated area
        region->size = region->allocated_size;
    }

    log_trace("Mapped on region %s virtual address %p-%p to physical %p-%p", 
        region->region_name, returning_addr, returning_addr + size, physical_start_addr, physical_start_addr + size);
    flush_if_required();
    // memory_management_region_dump(region);
    return returning_addr;
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

bool memory_management_attach(memory_t* memory, memory_region_t* region)
{
    if (!memory || !region) {
        log_error("Attaching invalid memory or region references. Mem=%p region=%p", memory, region);
        return false;
    }

    // verify if the region already exist
    if (linkedlist_find(memory->regions, region) != -1) {
        log_error("Region %s is already attached on memory %p", region->region_name, memory);
        return true;
    }

    if (linkedlist_append(memory->regions, region)) {
        if (linkedlist_append(region->attached, memory)) {

            // update the memory with all existed pages
            uintptr_t vaddr = region->start;
            WHILE_LINKEDLIST_ITER(region->pages, uintptr_t, paddr) {
                if (!add_new_memory_map(memory, region, paddr, vaddr)) {
                    goto error;
                }
                vaddr += SYSTEM_PAGE_SIZE;
            }
            return true;
        }
    }

error:
    log_error("Error attaching region %s on memory %p. Possible inconsistent state!", region->region_name, memory);
    return false;
}

memory_region_t* memory_management_region_create(memory_t* memory, const char* region_name, uintptr_t start, size_t size, bool user, bool writable, bool executable)
{
    if (!memory) {
        log_warn("Invalid memory reference to create a region");
        return NULL;
    }

    if (start == 0) {
        // next memory address available
        start = find_next_memory_address(memory);
    }

    if (MEM_ALIGN(start) != start) {
        log_warn("Invalid memory region %s, starting with an unaligned address: %p", region_name, start);
        return NULL;
    }

    // TODO There is any memory address constraint?
    // if (start < ???) {
    //  return NULL
    // }

    memory_region_t* region = NEW(memory_region_t);
    region->region_name = region_name;
    region->start = start;
    region->executable = executable;
    region->writable = writable;
    region->size = 0;  // allocates with size 0 and expand after.
    region->allocated_size = 0;
    region->user = user;
    region->pages = linkedlist_create();

    region->attached = linkedlist_create();
    if (!memory_management_attach(memory, region)) {
        log_error("Error attaching new region %s to memory %p", region->region_name, memory);
        return NULL;
    }

    memory_management_region_resize(region, size);
    log_trace("Created region %s of size 0x%x (%d KB) starting (virtual) at %p", region_name, size, size / 1024, start);
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

    bool failure = false;
    size_t new_allocated_size = ALIGN_NEXT(new_size, SYSTEM_PAGE_SIZE);
    if (new_allocated_size < region->allocated_size) {
        // TODO release pages not implemented yet
        log_warn("Shinking memory region %s not complete implemented", region->region_name);
        region->size = new_size;

    } else {
        // alocate all necessary pages
        size_t remain_space = new_allocated_size - region->allocated_size;
        while (remain_space > 0) {
            uintptr_t paddr = page_allocator_allocate();
            if (!paddr) {
                log_warn("No more memory for region %s", region->region_name);
                failure = true;
                break;
            }

            region_assoc_physical_address(region, paddr);
            remain_space -= SYSTEM_PAGE_SIZE;
        }

        // adjust the size only if there is enough allocated_size for it (in case of no more memory)
    }

    if (new_size <= region->allocated_size) {
        region->size = new_size;
    }
    flush_if_required();
    // memory_management_region_dump(region);
    return !failure;
}

void memory_management_dump(memory_t* memory)
{
    if (!memory) {
        return;
    }

    log_trace("====================== Dump memory =========================");
    log_trace("Memory addr: %p \t is kernel: %s", memory, memory == memory_management_get_kernel() ? "yes" : "no");
    log_trace("Num. regions: %d \t num. of maps: %d \t Next start address: %p", linkedlist_size(memory->regions), linkedlist_size(memory->map), memory->next_start_address);

    WHILE_LINKEDLIST_ITER(memory->regions, memory_region_t*, region) {
        memory_management_region_dump(region);
    }
    log_trace("------------------------------------------------------------");
}

void memory_management_region_dump(memory_region_t* region)
{
    if (!region) {
        log_warn("memory_management_region_dump called with NULL region");
        return;
    }

    log_trace("=========== Dump memory region ==============");
    log_trace("Name: %s \t Metadata: %p \t Attached: %d", region->region_name, region, linkedlist_size(region->attached));
    log_trace("Start Address: %p \t End (alloc): %p \t Size: %d (%d KB) bytes \t Allocated: %d KB", 
        region->start, region->start + region->allocated_size - 1, region->size, region->size / 1024, region->allocated_size / 1024);
    log_trace("Permissions: %c%c%c%c \t Physical pages: %d \t First Page Address: %p",
        region->user?'u':'-', 'r', region->writable?'w':'-', region->executable?'x':'-',
        linkedlist_size(region->pages), linkedlist_get(region->pages, 0)
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

size_t memory_management_region_current_size(memory_region_t* region)
{
    if (!region) {
        return 0;
    }
    return region->size;
}