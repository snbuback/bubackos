#include <kernel/page_allocator.h>
#include <kernel/memory_allocator.h>

/**
This implementation creates a vector of size total_memory/MEMORY_PAGE_SIZE of items
of memory_handler_t. Each slot with 0 means memory is free.
*/

volatile memory_handler_t* memory_mapping;

void mem_alloc_initialize()
{
}

memory_handler_t mem_alloc_create(void)
{

}

bool mem_alloc_release(memory_handler_t m_handler)
{

}

void* mem_alloc_allocate(memory_handler_t m_handler, size_t size)
{

}

void* mem_alloc_map(memory_handler_t m_handler, void* start, size_t size)
{

}
