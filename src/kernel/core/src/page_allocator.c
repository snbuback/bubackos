/**
 * Module responsible to manage the physical memory pages.
 */
#include <string.h>
#include <stdlib.h>
#include <hal/configuration.h>
#include <core/memory.h>
#include <core/page_allocator.h>
#include <logging.h>
#include <libutils/utils.h>

size_t total_of_pages;
size_t total_memory;
page_type* pages;

size_t page_allocator_page_number(uintptr_t aligned_addr)
{
    return aligned_addr / SYSTEM_PAGE_SIZE;
}

size_t page_allocator_next_page_available()
{
    // TODO search for page only after 64mb until ensure everything is safe
    for (size_t p = page_allocator_page_number(64*1024*1024); p < total_of_pages; ++p) {
        if (!pages[p]) {
            return p;
        }
    }
    return -1;
}

void page_allocator_initialize(size_t t)
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
}

static bool mark_page(uintptr_t align_addr, page_type page_type)
{
    size_t page_number = page_allocator_page_number(align_addr);
    if (pages[page_number]) {
        log_warn("Marking page ref to address %p as %d but page is %d", align_addr, page_type, pages[page_number]);
        return false;
    }
    pages[page_number] = PAGE_TYPE_USER;
    return true;
}

/*
 * Mark memory as system memory (kernel or reserved memory)
 */
bool page_allocator_mark_as_system(uintptr_t addr, size_t total_in_bytes)
{
    uintptr_t aligned_addr = ALIGN(addr, SYSTEM_PAGE_SIZE);
    size_t aligned_total = ALIGN_NEXT(total_in_bytes, SYSTEM_PAGE_SIZE);
    size_t remaining = aligned_total;
    while (remaining > 0) {
        mark_page(aligned_addr, PAGE_TYPE_SYSTEM);
        aligned_addr += SYSTEM_PAGE_SIZE;
        remaining -= SYSTEM_PAGE_SIZE;
    }
    log_debug("Marked addr %p of size %p as SYSTEM", (void*)aligned_addr, (void*)aligned_total);
    return true;
}

uintptr_t page_allocator_allocate()
{
    // TODO This function should be synchronized
    size_t page_number = page_allocator_next_page_available();
    pages[page_number] = PAGE_TYPE_USER;
    return page_number << SYSTEM_PAGE_ALIGN;
}

bool page_allocator_mark_as_free(uintptr_t addr)
{
    if (ALIGN(addr, SYSTEM_PAGE_SIZE) != addr) {
        log_error("Only use free memory of aligned address: %p", (void*)addr);
        return false;
    }
    return mark_page(addr, PAGE_TYPE_FREE);
}

page_type page_allocator_page_status(size_t page_number)
{
    return pages[page_number];
}

size_t page_allocator_get_total_pages()
{
    return total_of_pages;
}