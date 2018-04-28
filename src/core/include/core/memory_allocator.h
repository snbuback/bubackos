#ifndef __KERNEL_MEMORY_ALLOCATOR_H
#define __KERNEL_MEMORY_ALLOCATOR_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t memory_handler_t;

/**
Initialize the memory allocation module to the amount size of memory
*/
void mem_alloc_initialize();

/**
Creates a new memory allocation block
*/
memory_handler_t mem_alloc_create(void);

/**
Release a memory allocation block
*/
bool mem_alloc_release(memory_handler_t m_handler);

/**
Allocate/dealocate memory. If size is positive memory is allocate otherwise if
size is negative dealocate memory. Uses size 0 to return the highest memory address.
*/
void* mem_alloc_allocate(memory_handler_t m_handler, size_t size);

/**
Associate a physical address of memory to a memory m_handler
*/
void* mem_alloc_map(memory_handler_t m_handler, void* start, size_t size);

#endif
