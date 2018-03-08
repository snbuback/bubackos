#ifndef __KERNEL_SYSTEM_H
#define __KERNEL_SYSTEM_H

#include <kernel/gdt.h>
#include <kernel/memory_allocator.h>
#include <kernel/console.h>
#include <kernel/services.h>

// temp
void kprintf(const char *format, ...);
int multiboot_parser(uint64_t magic, uint64_t* addr);

#endif
