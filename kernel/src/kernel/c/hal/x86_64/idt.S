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
#include <hal/native_task.h>
#define NUM_SAVED_REGS 15
#define REGISTER_SIZE 	8

.extern interrupt_handler
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
