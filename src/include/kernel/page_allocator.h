#ifndef __KERNEL_PAGE_ALLOCATOR_H
#define __KERNEL_PAGE_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGE_TYPE_FREE    0
#define PAGE_TYPE_SYSTEM  1
#define PAGE_TYPE_USER    2

/* define by the linker */
extern uintptr_t kernel_virtual_start;
extern uintptr_t kernel_physical_start;
extern uintptr_t kernel_virtual_end;
extern uintptr_t kernel_physical_end;

typedef uint8_t page_type;

uintptr_t align(uintptr_t addr);

void page_allocator_initialize(size_t t);
bool page_allocator_mark_as_system(uintptr_t addr, size_t total_in_bytes);
uintptr_t page_allocator_allocate();
bool page_allocator_mark_as_free(uintptr_t addr);
page_type page_allocator_page_status(size_t page_number);
size_t page_allocator_get_total_pages();
#endif