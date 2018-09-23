/**
 * Module responsible to manage the physical memory pages.
 */
#include <string.h>
#include <stdlib.h>
#include <core/hal/platform.h>
#include <core/memory.h>
#include <core/pmem/services.h>
#include <logging.h>
#include <libutils/utils.h>

#define PAGE_TO_ADDR(p)     (p << SYSTEM_PAGE_ALIGN)
#define ADDR_TO_PAGE(a)     (a >> SYSTEM_PAGE_ALIGN)

size_t total_of_pages;
size_t total_memory;
page_type* pages;

bool pmem_initialize(size_t t)
{
    total_memory = t;
    total_of_pages = total_memory / SYSTEM_PAGE_SIZE;
    pages = (page_type*) kalloc(sizeof(page_type) * total_of_pages);

    log_trace("Page allocator data: start=%p end=%p total_of_pages=%d page size=%d", (void*)pages, (void*)pages + (total_of_pages + 1), total_of_pages, SYSTEM_PAGE_SIZE);

    // mark all pages as free
    for (size_t p = 0; p < total_of_pages; ++p) {
        pages[p] = PAGE_TYPE_FREE;
    }
    log_debug("Page Allocator initialized");
    return true;
}

static inline bool pmem_set_page(uintptr_t page_number, page_type page_type)
{
    if (page_number >= total_of_pages) {
        log_error("Invalid memory reference at %p", PAGE_TO_ADDR(page_number));
        return false;
    }
    // SYSTEM pages are immutable.
    if (pages[page_number] != PAGE_TYPE_SYSTEM) {
        pages[page_number] = page_type;
    }
    return true;
}

/**
 * Returns the number of the first page with status pagetype.
 */
static size_t pmem_next_page_available(page_type pagetype)
{
    // TODO search for page only after 64mb until ensure everything is safe
    for (size_t p = ADDR_TO_PAGE(64*1024*1024); p < total_of_pages; ++p) {
        if (pages[p] == pagetype) {
            return p;
        }
    }
    log_error("No memory page of type %d available.", pagetype);
    return 0;
}

uintptr_t pmem_allocate()
{
    // TODO This function should be synchronized
    size_t page_number = pmem_next_page_available(PAGE_TYPE_FREE);
    if (!page_number) {
        goto error;
    }
    if (!pmem_set_page(page_number, PAGE_TYPE_USER)) {
        goto error;
    }
    return PAGE_TO_ADDR(page_number);

error:
    log_error("Error allocating physical memory");
    return 0;
}


/*
 * Mark memory as system memory (kernel or reserved memory)
 */
bool pmem_set_as_system(uintptr_t addr, size_t total_in_bytes)
{
    uintptr_t page = ADDR_TO_PAGE(addr);
    for (size_t remaining = ADDR_TO_PAGE(ALIGN_NEXT(total_in_bytes, SYSTEM_PAGE_SIZE)); remaining > 0; --remaining) {
        // ignore errors, try to mark maximum pages as possible.
        pmem_set_page(page, PAGE_TYPE_SYSTEM);
    }
    return true;
}

bool pmem_release(uintptr_t addr)
{
    if (ALIGN(addr, SYSTEM_PAGE_SIZE) != addr) {
        log_error("Only use free with aligned address: %p", addr);
        return false;
    }
    return pmem_set_page(ADDR_TO_PAGE(addr), PAGE_TYPE_FREE);
}
