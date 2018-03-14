#include <stddef.h>
#include <unistd.h>
#include <kernel/configuration.h>
#include <kernel/page_allocator.h>

#include "src/kernel/page_allocator.cpp"

void test_allocate_and_deallocate()
{
    const size_t total_of_memory = 1024 * 1024 * 1024;
    const size_t total_of_pages = total_of_memory / SYSTEM_PAGE_SIZE;

    // to avoid the page allocator write in a invalid page of memory, the kernel should start at the data segment
    const uintptr_t kernel_page_start = reinterpret_cast<uintptr_t>(malloc(1))/SYSTEM_PAGE_SIZE;
    const uintptr_t kernel_page_end = kernel_page_start+17;
    const uintptr_t page_allocator_end = (kernel_page_end + 1) * SYSTEM_PAGE_SIZE + (total_of_pages * sizeof(page_type));

    // alocate memory to cheap the page allocator running as a regular user process
    void* test_memory = 0; //malloc(page_allocator_end * SYSTEM_PAGE_SIZE);
    printf("Total memory for this test=%p total_of_pages=%ld page_allocator_end=%p\n", test_memory, total_of_pages, (void*) page_allocator_end);
    PageAllocator pa = PageAllocator(total_of_memory, kernel_page_start * SYSTEM_PAGE_SIZE, kernel_page_end * SYSTEM_PAGE_SIZE);
    printf("aqui %ld\n\n ", total_of_pages);
    ASSERT(pa.get_total_pages() == total_of_pages, "Invalid total of pages");
    for (size_t p = 0; p < total_of_pages; ++p)
    {
        if (p < kernel_page_start || p > page_allocator_end) {
            ASSERT(pa.page_status(p) == PAGE_TYPE_SYSTEM, "Page not marked as system");
        } else {
            ASSERT(pa.page_status(p) == PAGE_TYPE_FREE, "Page not marked as free");
        }
    }
}
