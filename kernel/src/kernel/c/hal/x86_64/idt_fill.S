#include <hal/idt.h>

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
	interrupt 19
	interrupt 20
	interrupt 21
	interrupt 22
	interrupt 23
	interrupt 24
	interrupt 25
	interrupt 26
	interrupt 27
	interrupt 28
	interrupt 29
	interrupt 30
	interrupt 31
	interrupt 32
	interrupt 33
	interrupt 34
	interrupt 35
	interrupt 36
	interrupt 37
	interrupt 38
	interrupt 39
	interrupt 40
	interrupt 41
	interrupt 42
	interrupt 43
	interrupt 44
	interrupt 45
	interrupt 46
	interrupt 47
	interrupt 48
	interrupt 49

/* void idt_fill_table(int num_int); */
.globl idt_fill_table
idt_fill_table:
	enter $0, $0

    

#define REGISTER_INTR(n)    movq $INTERRUPT_GATE_386, %rdx; movq $n, %rdi; movq $intr ## n, %rsi ; call idt_set_gate

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
	REGISTER_INTR(19)
	REGISTER_INTR(20)
	REGISTER_INTR(21)
	REGISTER_INTR(22)
	REGISTER_INTR(23)
	REGISTER_INTR(24)
	REGISTER_INTR(25)
	REGISTER_INTR(26)
	REGISTER_INTR(27)
	REGISTER_INTR(28)
	REGISTER_INTR(29)
	REGISTER_INTR(30)
	REGISTER_INTR(31)
	REGISTER_INTR(32)
	REGISTER_INTR(33)
	REGISTER_INTR(34)
	REGISTER_INTR(35)
	REGISTER_INTR(36)
	REGISTER_INTR(37)
	REGISTER_INTR(38)
	REGISTER_INTR(39)
	REGISTER_INTR(40)
	REGISTER_INTR(41)
	REGISTER_INTR(42)
	REGISTER_INTR(43)
	REGISTER_INTR(44)
	REGISTER_INTR(45)
	REGISTER_INTR(46)
	REGISTER_INTR(47)
	REGISTER_INTR(48)
	REGISTER_INTR(49)

	leave
	ret
