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
#include <hal/idt.h>
#include <hal/gdt.h>
#include <hal/native_task.h>
#define NUM_SAVED_REGS 15
#define REGISTER_SIZE 	8

.extern interrupt_handler
.extern do_syscall
.extern do_task_switch
.extern task_update_current_state
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


/**
 * Intel manual Vol. 3A 5-23
 */
.globl syscall_install
syscall_install:
    // rdi pointer to the stack address
    pushq   %rbp
    movq    %rsp, %rbp

    // stores the target address
    // wrmsrl(IA32_LSTAR=0xC0000082, system_call);
    // pointer address split between rdx:rax
    movq    $.syscall_jumper, %rax      // high order bits are ignored
    movq    $.syscall_jumper, %rdx
    shr     $32, %rdx
    mov     $0xC0000082, %rcx
    wrmsr

    // stores the cs segment from kernel and user space
    // wrmsrl(IA32_STAR=0xC0000082, system_call);
    // rdx contains the kernel and user segment selector
    movq    $0, %rax
    movq    $(GDT_SEGMENT(GDT_ENTRY_USER_CS)), %rdx
    shl     $16, %rdx
    orq     $(GDT_SEGMENT(GDT_ENTRY_KERNEL_CS)), %rdx
    mov     $0xC0000081, %rcx
    wrmsr

    // IA32_FMASK: kernel rflags mask
    movq    $0, %rax // kernel flags mask (will zero current flags register on set bits)
    movq    $0, %rdx
    mov     $0xC0000084, %rcx
    wrmsr

    // IA32_KERNEL_GS_BASE: kernel stack pointer
    movq    %rdi, %rax      // high order bits are ignored
    movq    %rdi, %rdx
    shr     $32,  %rdx
    mov     $0xC0000102, %rcx
    wrmsr

    // IA32_EFER: enable syscall
    mov   $0xC0000080, %rcx
    rdmsr /* takes ecx as argument, deposits contents of MSR into eax */
    or    $0b000000001, %rax
    wrmsr /* exactly the reverse of rdmsr */

    leave
    ret
    

.syscall_jumper:
    cli
    swapgs
    /* save user stack pointer. Since rax is the return of syscall there is no problem in change it's value */
    movq    %rsp, %rax
    movq    %gs:0, %rsp

    pushq   $(GDT_SEGMENT(GDT_ENTRY_USER_DS)+GDT_RING_USER) // ss
    pushq   %rax    // user's rsp
    pushq   %r11    // r11 (contains rflags after sysenter)
    pushq   $(GDT_SEGMENT(GDT_ENTRY_USER_CS)+GDT_RING_USER) // cs
    pushq   %rcx    // user's next instruction
    pushq   $0

    hal_save_task_state()

    // apart from exit syscall, all other requires the save state
    movq    %rsp, %rdi
    call    task_update_current_state

    movq    14*8(%rdi), %rsi
    call    do_syscall
    call    do_task_switch
    // this call never returns
