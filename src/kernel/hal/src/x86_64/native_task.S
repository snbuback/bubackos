#include <hal/configuration.h>
#include <x86_64/gdt.h>
#include <x86_64/native_task.h>
#define REG_RESTORE(reg, pos)       movq 8*pos(%rdi), %reg
#define REG_ENTRY(n)                8*n(%rdi)

.section .text
.code64

/* void hal_create_native_task(native_task_t *task, uintptr_t code, uintptr_t stack, int permission_mode); */
.globl hal_create_native_task
hal_create_native_task:
    enter $0, $0

    movq    %rdx, REG_ENTRY(4)      // rbp
    movq    %rdx, REG_ENTRY(19)     // rsp
    movq    %rsi, REG_ENTRY(16)     // code
    movq    $INITIAL_EFLAGS, REG_ENTRY(18)  // eflags

    jrcxz   1f
    // kernel mode
    movq    $(GDT_SEGMENT(GDT_ENTRY_KERNEL_CS)+GDT_RING_SYSTEM), REG_ENTRY(17)     // user code segment with bottom 2 bits set for ring 3
    movq    $(GDT_SEGMENT(GDT_ENTRY_KERNEL_DS)+GDT_RING_SYSTEM), REG_ENTRY(20)     // user data segment with bottom 2 bits set for ring 3
    jmp     2f
1:
    // user mode
    movq    $(GDT_SEGMENT(GDT_ENTRY_USER_CS)+GDT_RING_USER), REG_ENTRY(17)     // user code segment with bottom 2 bits set for ring 3
    movq    $(GDT_SEGMENT(GDT_ENTRY_USER_DS)+GDT_RING_USER), REG_ENTRY(20)     // user data segment with bottom 2 bits set for ring 3

2:
    leave
    ret


// hal_save_task_state: implementation in native_task.h


// void hal_switch_task(native_task_t *task)  __attribute__ ((noreturn));
.globl hal_switch_task
hal_switch_task:
    // first, prepare the stack to iret, so we don't need to worry about the register values
    pushq   REG_ENTRY(20)
    pushq   REG_ENTRY(19)
    pushq   REG_ENTRY(18)
    pushq   REG_ENTRY(17)
    pushq   REG_ENTRY(16)

    // change other register segments
    movq    $(GDT_SEGMENT(GDT_ENTRY_USER_DS)+GDT_RING_USER), %rax
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

.globl hal_sleep
hal_sleep:
    # adjust kernel stack to the begin
    mov     $cpu0_kernel_stack, %rbp
    addq    $SYSTEM_STACKSIZE, %rbp
    mov     %rbp, %rsp

    sti
    hlt
    jmp hal_sleep