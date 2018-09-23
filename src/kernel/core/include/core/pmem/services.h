#ifndef _CORE_PMEM_SERVICES_H_
#define _CORE_PMEM_SERVICES_H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGE_TYPE_FREE    0
#define PAGE_TYPE_SYSTEM  1
#define PAGE_TYPE_USER    2

typedef uint8_t page_type;

bool pmem_initialize();

/**
 * Allocates a page of physical memory.
 */
uintptr_t pmem_allocate();

/**
 * Mark a address of memory as system.
 * A system page address could not be allocated manually and after associate with a page of memory
 * is not mark as free. Will be SYSTEM forever.
 */
bool pmem_set_as_system(uintptr_t addr, size_t total_in_bytes);

/**
 * Marks a page of memory starting from addr as free.
 * If the page was marked as SYSTEM, the status doesn't change.
 */
bool pmem_release(uintptr_t addr);

#endif