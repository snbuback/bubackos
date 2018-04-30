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

#define NUM_SAVED_REGS 15
#define REGISTER_SIZE 	8

.text
.code64
	.extern interrupt_handler

.macro interrupt num:req, errorcode=1
.global intr\num
intr\num\():
.if \errorcode
	pushq $0
.endif
	pushq $\num
	jmp intr_handler
.endm


	interrupt 0
	interrupt 1
	interrupt 2
	interrupt 3
	interrupt 4
	interrupt 5
	interrupt 6
	interrupt 7
	interrupt 8
	interrupt 9
	interrupt 10, 0
	interrupt 11, 0
	interrupt 12, 0
	interrupt 13, 0
	interrupt 14, 0
	interrupt 15
	interrupt 16
	interrupt 17, 0
	interrupt 18


# List of registers from here: https://github.com/torvalds/linux/blob/v4.8/arch/x86/include/uapi/asm/ptrace.h
# Remember that the stack grows from bottom to top
intr_handler:
	pushq %rax;
	pushq %rbx;
	pushq %rcx;
	pushq %rdx;
	pushq %r8;
	pushq %r9;
	pushq %r10;
	pushq %r11;
	pushq %r12;
	pushq %r13;
	pushq %r14;
	pushq %r15;
	pushq %rbp;
	pushq %rsi;
	pushq %rdi;

	# movq	0x10, %rax	# switch to kernel data seg
	# movq	%rax, %ds
	movq	((NUM_SAVED_REGS)*REGISTER_SIZE)(%rsp), %rdi
	movq	((NUM_SAVED_REGS+1)*REGISTER_SIZE)(%rsp), %rsi
	call	interrupt_handler

	# remove the interruption number and code
	addq $(REGISTER_SIZE*2), %rsp

	// ack int
	movb $0x20, %al
	outb %al, $0x20

	// restore and return
	popq %rdi;
	popq %rsi;
	popq %rbp;
	popq %r15;
	popq %r14;
	popq %r13;
	popq %r12;
	popq %r11;
	popq %r10;
	popq %r9;
	popq %r8;
	popq %rdx;
	popq %rcx;
	popq %rbx;
	popq %rax;
	iretq

/* void idt_flush(uintptr_t base, uint16_t limit); */
.globl idt_flush
idt_flush:
  enter $10, $0
  movq  %rsi, -10(%rbp)
  movq  %rdi, -8(%rbp)
  lidt (%rsp)
  leave
  ret

