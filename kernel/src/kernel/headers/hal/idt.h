#ifndef __KERNEL_IDT_H
#define __KERNEL_IDT_H
#include <stdbool.h>

void idt_install();
bool are_interrupts_enabled();
void idt_flush(uintptr_t base, uint16_t limit);
#endif