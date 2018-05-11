#ifndef __HAL_IDT_H
#define __HAL_IDT_H

#define IDT_TOTAL_INTERRUPTIONS     51

#define TASK_GATE_286           0x5
#define INTERRUPT_GATE_286      0x6
#define TRAP_GATE_286           0x7
#define INTERRUPT_GATE_386      0xE
#define TRAP_GATE_386           0xF

#define INT_SYSTEM_CALL   50      // 0x32

#ifndef ASM_FILE
#include <stdint.h>
#include <hal/native_task.h>

/* Defines an IDT entry */
typedef struct {
    unsigned base_0_15 : 16;
    unsigned segment : 16;
    unsigned ist : 3;
    unsigned : 5;
    unsigned type : 4;
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


void idt_initialize();
void idt_install();
void syscall_install(uintptr_t stack);
void idt_set_gate(unsigned num, uintptr_t base, unsigned type, unsigned ring);
void interrupt_handler(native_task_t *native_task, int interrupt) __attribute__ ((noreturn)); // called by assembly functions

// assembly functions
void idt_fill_table();
void idt_flush(uintptr_t base, uint16_t limit);

#endif // ASM_FILE
#endif