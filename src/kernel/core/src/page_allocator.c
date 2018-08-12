/**
 * Module responsible to manage the physical memory pages.
 */
#include <string.h>
#include <stdlib.h>
#include <hal/configuration.h>
#include <core/memory.h>
#include <core/page_allocator.h>
#include <logging.h>

size_t total_of_pages;
size_t total_memory;
page_type* pages;

uintptr_t align(uintptr_t addr)
{
    return addr & ~(uintptr_t)(SYSTEM_PAGE_ALIGN - 1);
}

size_t page_allocator_page_number(uintptr_t aligned_addr)
{
    return aligned_addr / SYSTEM_PAGE_SIZE;
}

size_t page_allocator_next_page_available()
{
    for (size_t p = 0; p < total_of_pages; ++p) {
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
    pages = (page_type*) malloc(sizeof(page_type) * total_of_pages);

    log_trace("Page allocator data: start=%p end=%p total_of_pages=%d page size=%d", (void*)pages, (void*)pages + (total_of_pages + 1), total_of_pages, SYSTEM_PAGE_SIZE);

    // mark all pages as free
    for (size_t p = 0; p < total_of_pages; ++p) {
        pages[p] = PAGE_TYPE_FREE;
    }

    // TODO Keeping this logic until ensure these areas are ok to use
    // lock the first 4mb
    for (int i=0; i<4*1024*1024 / SYSTEM_PAGE_SIZE; ++i) {
        pages[i] = PAGE_TYPE_SYSTEM;
    }

    log_debug("Page Allocator initialized");

}

/*
 * Mark memory as system memory (kernel or reserved memory)
 */
bool page_allocator_mark_as_system(uintptr_t addr, size_t total_in_bytes)
{
    uintptr_t aligned_addr = align(addr);
    size_t aligned_total = (total_in_bytes + SYSTEM_PAGE_ALIGN - 1) & ~(SYSTEM_PAGE_ALIGN - 1);
    pages[page_allocator_page_number(aligned_addr)] = PAGE_TYPE_SYSTEM;
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
    if (align(addr) != addr) {
        log_error("Only use free memory of aligned address: %p", (void*)addr);
    }
    pages[page_allocator_page_number(addr)] = PAGE_TYPE_FREE;
    return true;
}

page_type page_allocator_page_status(size_t page_number)
{
    return pages[page_number];
}

size_t page_allocator_get_total_pages()
{
    return total_of_pages;
}