#include <core/hal/platform.h>
#include <x86_64/gdt.h>
#include <x86_64/native_task.h>
#define REG_RESTORE(reg, pos)       movq 8*pos(%rdi), %reg
#define REG_ENTRY(n)                8*n(%rdi)

.section .text
.code64

// TODO Stop using pushq. It is make impossible use execution protection on kernel memory data (see kalloc)
// void intel_switch_task(native_task_t *task)  __attribute__ ((noreturn));
.globl intel_switch_task
intel_switch_task:
    // first, prepare the stack to iret, so we don't need to worry about the register values
    pushq   REG_ENTRY(20)
    pushq   REG_ENTRY(19)
    pushq   REG_ENTRY(18)
    pushq   REG_ENTRY(17)
    pushq   REG_ENTRY(16)

    // change other register segments
    // movq    $(GDT_SEGMENT(GDT_ENTRY_USER_DS)+GDT_RING_USER), %rax
    movq    $0, %rax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs
    //we don't need to worry about SS. it's handled by iret

    //  TODO Improvement: controls the stack and change the instructions to use pop instead of mov (faster)
    // now, register all register, but SS, rsp, rip (pointer), rflags
    REG_RESTORE(r15, 0)
    REG_RESTORE(r14, 1)
    REG_RESTORE(r13, 2)
    REG_RESTORE(r12, 3)
    REG_RESTORE(rbp, 4)
    REG_RESTORE(rbx, 5)
    REG_RESTORE(r11, 6)
    REG_RESTORE(r10, 7)
    REG_RESTORE(r9,  8)
    REG_RESTORE(r8,  9)
    REG_RESTORE(rax, 10)
    REG_RESTORE(rcx, 11)
    REG_RESTORE(rdx, 12)
    REG_RESTORE(rsi, 13)
    REG_RESTORE(rdi, 14) // last to restore because pointer to all information
    iretq

.globl native_task_sleep
native_task_sleep:
    sti
    hlt
    jmp native_task_sleep