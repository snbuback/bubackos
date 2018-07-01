#ifndef __KERNEL_PAGE_ALLOCATOR_H
#define __KERNEL_PAGE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGE_TYPE_FREE    0
#define PAGE_TYPE_SYSTEM  1
#define PAGE_TYPE_USER    2

typedef uint8_t page_type;

uintptr_t align(uintptr_t addr);

void page_allocator_initialize();
bool page_allocator_mark_as_system(uintptr_t addr, size_t total_in_bytes);
uintptr_t page_allocator_allocate();
bool page_allocator_mark_as_free(uintptr_t addr);
page_type page_allocator_page_status(size_t page_number);
size_t page_allocator_get_total_pages();
#endif