#ifndef __HAL_IDT_H
#define __HAL_IDT_H
#include <stdint.h>

typedef enum
{
	TASK_GATE_286 = 0x5,
	INTERRUPT_GATE_286 = 0x6,
	TRAP_GATE_286 = 0x7,
	INTERRUPT_GATE_386 = 0xE,
	TRAP_GATE_386 = 0xF
} enum_gate_type;

/* Defines an IDT entry */
typedef struct {
    unsigned base_0_15 : 16;
    unsigned segment : 16;
    unsigned ist : 3;
    unsigned : 5;
    enum_gate_type type : 4;
    unsigned : 1;
    unsigned ring : 2;
    unsigned present : 1;
    unsigned base_16_31 : 16;
    unsigned base_32_63 : 32;
    unsigned : 32;
} __attribute__((packed)) idt_entry;

typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr;

void idt_install();

// assembly functions
void idt_flush(uintptr_t base, uint16_t limit);

#endif