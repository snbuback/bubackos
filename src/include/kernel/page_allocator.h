#ifndef __KERNEL_PAGE_ALLOCATOR_H
#define __KERNEL_PAGE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

enum page_type : uint8_t
{
    PAGE_TYPE_FREE,
    PAGE_TYPE_SYSTEM,
    PAGE_TYPE_USER
};

class PageAllocator
{
  private:
    size_t total_memory;
    uint16_t total_of_pages;
    page_type* pages;

    uintptr_t align(uintptr_t addr);

    size_t page_number(uintptr_t aligned_addr);

    size_t next_page_available();

  public:
    PageAllocator(size_t t, uintptr_t kernel_start_address, uintptr_t kernel_end_address);

    /**
     * Mark memory as system memory (kernel or reserved memory)
     */
    bool mark_as_system(uintptr_t addr, size_t total_in_bytes);

    uintptr_t allocate();

    bool mark_as_free(uintptr_t addr);

    page_type page_status(size_t page_number);

    size_t get_total_pages();
};

#endif