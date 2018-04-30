#define NUM_SAVED_REGS 15
#define REGISTER_SIZE 	8

	.text
	.code64
	.extern interrupt_handler

#
	.global int_doublefault
	.extern int_doublefault_handler
int_doublefault:
	call int_doublefault_handler
	iretq

	.global int_generalfault
	.extern int_generalfault_handler
int_generalfault:
	call int_generalfault_handler
	iretq

	.global int_test
	.extern hi
int_test:

	/* save registers */
	/*push %rax
	push %rcx
	push %rdx
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11*/

	/*movq (9*8)(%rsp), %rsi
	movq %rsp, %rdi
	addq $(10*8), %rdi
	subq $8, %rsp*/
	call hi
	/*add $8, %rsp*/

	/* restore */
	/*pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rdx
	pop %rcx
	pop %rax*/

/*	add $8, %rsp
	add $8, %rsp
	add $8, %rsp*/
	xchgw %bx, %bx
	iretq


/*global InvalidTSS
extern HandleIT
InvalidTSS:
	call HandleIT
	iret

global SegmentNotPresent
extern HandleSNP
SegmentNotPresent:
	call HandleSNP
	iret

global GeneralProtectionFault
extern HandleGPF
GeneralProtectionFault:
	call HandleGPF
	iret

global PageFault
extern HandlePF
PageFault:
	mov eax, cr2
	mov [regInfo], eax
	call HandlePF
	iret
*/

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

	# remove the interruption number and code
	addq $(REGISTER_SIZE*2), %rsp
	movb $0x20, %al
	outb %al, $0x20
	# iret restore rip, cs, eflags, rsp, ss
	iretq

