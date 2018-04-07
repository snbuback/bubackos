#include <kernel/page_allocator.h>
#include <kernel/configuration.h>
#include <kernel/logging.h>
#include <string.h>

size_t total_of_pages;
size_t total_memory;
// TODO FIXME the maximum page size
page_type pages[32768];  // 128 MB with 4KB page size

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
    for (size_t p = 0; p < total_of_pages; ++p)
    {
        if (!pages[p])
        {
            return p;
        }
    }
    return -1;
}

void page_allocator_initialize()
{
    // TODO FIXME this code
    size_t t = 128*1024*1024;
    uintptr_t kernel_start_address = 0;//kernel_physical_start;
    // uintptr_t kernel_start_address = &page_allocator_initialize;
    uintptr_t kernel_end_address = 0;//kernel_virtual_end;
    
    total_memory = t;
    total_of_pages = total_memory / SYSTEM_PAGE_SIZE;

    // remove this code from here
    log_debug("Page size = %d bytes (align %d) total pages=%d", (int) SYSTEM_PAGE_SIZE, (int) SYSTEM_PAGE_ALIGN, (int) total_of_pages);
    log_debug("Kernel from %p to %p", (void*) kernel_start_address, (void*) kernel_end_address);
    log_debug("Kernel (aligned) from %p to %p", (void*) align(kernel_start_address), (void*) align(kernel_end_address));

    size_t kernel_page_start = page_allocator_page_number(align(kernel_start_address));
    size_t kernel_page_end = page_allocator_page_number(align(kernel_end_address));
    log_debug("Kernel page number from %d to %d", (int) kernel_page_start, (int) kernel_page_end);

    // the page allocator memory are stored after the kernel pages.
    size_t pa_page_start = kernel_page_end + 1;
    size_t pa_size = total_of_pages * sizeof(page_type);
    size_t pa_page_end = pa_page_start + total_of_pages;

    // pages = (page_type*) (pa_page_start * SYSTEM_PAGE_SIZE);

    log_debug("Page allocator data: start=%p end=%p size=%d bytes", (void*) pages, (void*) pages + total_of_pages, (int) pa_size);
    memset(pages, 0x0, pa_size);

    log_debug("kernel_page_start=%d pa_page_end=%d", (int) kernel_page_start, (int) pa_page_end);
    for (size_t p = kernel_page_start; p < pa_page_end; ++p)
    {
        pages[p] = PAGE_TYPE_SYSTEM;
    }
    log_debug("Page Allocator initialized");
}

/**
     * Mark memory as system memory (kernel or reserved memory)
     */
bool page_allocator_mark_as_system(uintptr_t addr, size_t total_in_bytes)
{
    uintptr_t aligned_addr = align(addr);
    size_t aligned_total = (total_in_bytes + SYSTEM_PAGE_ALIGN - 1) & ~(SYSTEM_PAGE_ALIGN - 1);
    pages[page_allocator_page_number(aligned_addr)] = PAGE_TYPE_SYSTEM;
    log_debug("Marked addr %p of size %p as SYSTEM", (void*) aligned_addr, (void*) aligned_total);
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
    if (align(addr) != addr)
    {
        log_error("Only use free memory of aligned address: %p", (void*) addr);
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