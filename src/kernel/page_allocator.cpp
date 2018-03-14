#include <kernel/page_allocator.h>
#include <kernel/configuration.h>
#include <kernel/logging.h>
#include <string.h>

uintptr_t PageAllocator::align(uintptr_t addr)
{
    return addr & ~(SYSTEM_PAGE_ALIGN - 1);
}

size_t PageAllocator::page_number(uintptr_t aligned_addr)
{
    return aligned_addr >> (SYSTEM_PAGE_ALIGN + 1);
}

size_t PageAllocator::next_page_available()
{
    for (int p = 0; p < total_of_pages; ++p)
    {
        if (!pages[p])
        {
            return p;
        }
    }
    return -1;
}

PageAllocator::PageAllocator(size_t t, uintptr_t kernel_start_address, uintptr_t kernel_end_address)
{
    total_memory = t;
    total_of_pages = total_memory / SYSTEM_PAGE_SIZE;

    LOG_DEBUG("Kernel from 0x%lx to 0x%lx (aligned from 0x%lx to 0x%lx to page size of %ld)", kernel_start_address, kernel_end_address, align(kernel_start_address), align(kernel_end_address), (int) SYSTEM_PAGE_SIZE);

    size_t kernel_page_start = page_number(align(kernel_start_address));
    size_t kernel_page_end = page_number(align(kernel_end_address));
    LOG_DEBUG("k s=%d kernel_page_end=%d", kernel_page_start, kernel_page_end);

    // the page allocator memory are stored after the kernel pages.
    size_t pa_page_start = kernel_page_end + 1;
    size_t pa_size = total_of_pages * sizeof(page_type);
    size_t pa_page_end = pa_page_start + total_of_pages;

    pages = reinterpret_cast<page_type *>(pa_page_start * SYSTEM_PAGE_SIZE);

    LOG_DEBUG("Page allocator data: start=0x%x end=0x%x size=%d bytes", pages, pages + total_of_pages, pa_size);
    memset(pages, 0x0, pa_size);

    LOG_DEBUG("kernel_page_start=%d pa_page_end=%d\n", kernel_page_start, pa_page_end);
    for (size_t p = kernel_page_start; p < pa_page_end; ++p)
    {
        LOG_DEBUG("assign p=%ld address=0x%x", p, pages + p);
        pages[p] = PAGE_TYPE_SYSTEM;
    }
}

/**
     * Mark memory as system memory (kernel or reserved memory)
     */
bool PageAllocator::mark_as_system(uintptr_t addr, size_t total_in_bytes)
{
    uintptr_t aligned_addr = align(addr);
    size_t aligned_total = (total_in_bytes + SYSTEM_PAGE_ALIGN - 1) & ~(SYSTEM_PAGE_ALIGN - 1);
    pages[page_number(aligned_addr)] = PAGE_TYPE_SYSTEM;
    LOG_DEBUG("Marked addr 0x%x of size %x as SYSTEM", aligned_addr, aligned_total);
    return true;
}

uintptr_t PageAllocator::allocate()
{
    // TODO This function should be synchronized
    size_t page_number = next_page_available();
    pages[page_number] = PAGE_TYPE_USER;
    return page_number << SYSTEM_PAGE_ALIGN;
}

bool PageAllocator::mark_as_free(uintptr_t addr)
{
    if (align(addr) != addr)
    {
        LOG_ERROR("Only use free memory of aligned address: 0x%x", addr);
    }
    pages[page_number(addr)] = PAGE_TYPE_FREE;
    return true;
}

page_type PageAllocator::page_status(size_t page_number)
{
    return pages[page_number];
}

size_t PageAllocator::get_total_pages()
{
    return total_of_pages;
}