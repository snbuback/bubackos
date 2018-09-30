/* About arguments here: https://www.nasm.us/doc/nasmdo11.html and http://cons.mit.edu/fa17/x86-64-architecture-guide.html*/
/* Summary:
Register order: RDI, RSI, RDX, RCX, R8, R9
Return in RAX

Register to save: RBX, RBP, and R12–R15

func:
    pushq	%rbp
    movq	%rsp, %rbp
  ...
  leave
  ret

*/
#include <x86_64/idt.h>
#include <x86_64/gdt.h>
#include <x86_64/native_task.h>
#define NUM_SAVED_REGS 15
#define REGISTER_SIZE 	8

.extern interrupt_handler
.extern do_syscall
.extern scheduler_switch_task
.extern hal_update_current_state
.text
.code64

/**
* %rdi pointer to native_task_t
* %rsi interruption number
*/
.globl intr_handler
intr_handler:
    jmp		interrupt_handler


/* void idt_flush(uintptr_t base, uint16_t limit); */
.globl idt_flush
idt_flush:
    enter $10, $0
    movq  %rsi, -10(%rbp)
    movq  %rdi, -8(%rbp)
    lidt (%rsp)
    leave
    ret

.globl syscall_jumper
syscall_jumper:
    // SYSCALL loads CS from STAR 47:32. It masks EFLAGS with SFMASK.
    // Instructions: Next it stores EIP in ECX. It then loads EIP from STAR 32:0 and SS from STAR 47:32 + 8. It then executes.
    cli
    swapgs
    /* save user stack pointer. Since rax is the return of syscall there is no problem in change it's value */
    movq    %rsp, %rax
    movq    %gs:0, %rsp
    swapgs  // to ensure the value of IA32_KERNEL_GS_BASE remaining the same even without a return from here.

    pushq   $(GDT_SEGMENT(GDT_ENTRY_USER_DS)+GDT_RING_USER) // ss --- overriden by hal_update_current_state
    pushq   %rax    // user's rsp
    pushq   %r11    // r11 (contains rflags after sysenter)
    pushq   $(GDT_SEGMENT(GDT_ENTRY_USER_CS)+GDT_RING_USER) // cs --- overriden by hal_update_current_state
    pushq   %rcx    // user's next instruction
    pushq   $0

    // first the registers are save into the stack. In the hal_update_current_state
    // the registers are copied to the native_task associate to the task
    hal_save_task_state()
    // TODO clear rbp, rax, because contains kernel memory addresses

    // the native_task* pointer is the argument
    movq    %rsp, %rdi
    // returns the pointer to the kernel native_task. This is the return of this function
    call    hal_update_current_state

    // rax contains the pointer to task native_task
    movq    %rax, %rdi

    // TODO is necessary fix some incorrect registers, because were overriden by kernel (like rax with kernel stack address)

    call    pre_syscall

    // rax contains point to native_task
    movq    %rax, %rdi
    call    intel_switch_task
