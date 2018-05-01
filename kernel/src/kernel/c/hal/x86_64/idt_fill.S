#define IDT_TOTAL_INTERRUPTIONS 18

.extern idt_set_gate
.extern intr_handler

.text
.code64

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

/* void idt_fill_table(int num_int); */
.globl idt_fill_table
idt_fill_table:
	enter $0, $0

    movq    $0xe, %rdx

#define REGISTER_INTR(n)    movq $n, %rdi; movq $intr ## n, %rsi ; call idt_set_gate

    REGISTER_INTR(0)
    REGISTER_INTR(1)
    REGISTER_INTR(2)
    REGISTER_INTR(3)
    REGISTER_INTR(4)
    REGISTER_INTR(5)
    REGISTER_INTR(6)
    REGISTER_INTR(7)
    REGISTER_INTR(8)
    REGISTER_INTR(9)
    REGISTER_INTR(10)
    REGISTER_INTR(11)
    REGISTER_INTR(12)
    REGISTER_INTR(13)
    REGISTER_INTR(14)
    REGISTER_INTR(15)
    REGISTER_INTR(16)
    REGISTER_INTR(17)
    REGISTER_INTR(18)

	leave
	ret
