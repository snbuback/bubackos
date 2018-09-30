#ifndef _HAL_NATIVE_TASK_H
#define _HAL_NATIVE_TASK_H

// https://github.com/torvalds/linux/blob/5aa90a84589282b87666f92b6c3c917c8080a9bf/arch/x86/include/uapi/asm/processor-flags.h
#define _BITUL(x)	(1 << (x))
/*
 * EFLAGS bits
 */
#define X86_EFLAGS_CF_BIT	0 /* Carry Flag */
#define X86_EFLAGS_CF		_BITUL(X86_EFLAGS_CF_BIT)
#define X86_EFLAGS_FIXED_BIT	1 /* Bit 1 - always on */
#define X86_EFLAGS_FIXED	_BITUL(X86_EFLAGS_FIXED_BIT)
#define X86_EFLAGS_PF_BIT	2 /* Parity Flag */
#define X86_EFLAGS_PF		_BITUL(X86_EFLAGS_PF_BIT)
#define X86_EFLAGS_AF_BIT	4 /* Auxiliary carry Flag */
#define X86_EFLAGS_AF		_BITUL(X86_EFLAGS_AF_BIT)
#define X86_EFLAGS_ZF_BIT	6 /* Zero Flag */
#define X86_EFLAGS_ZF		_BITUL(X86_EFLAGS_ZF_BIT)
#define X86_EFLAGS_SF_BIT	7 /* Sign Flag */
#define X86_EFLAGS_SF		_BITUL(X86_EFLAGS_SF_BIT)
#define X86_EFLAGS_TF_BIT	8 /* Trap Flag */
#define X86_EFLAGS_TF		_BITUL(X86_EFLAGS_TF_BIT)
#define X86_EFLAGS_IF_BIT	9 /* Interrupt Flag */
#define X86_EFLAGS_IF		_BITUL(X86_EFLAGS_IF_BIT)
#define X86_EFLAGS_DF_BIT	10 /* Direction Flag */
#define X86_EFLAGS_DF		_BITUL(X86_EFLAGS_DF_BIT)
#define X86_EFLAGS_OF_BIT	11 /* Overflow Flag */
#define X86_EFLAGS_OF		_BITUL(X86_EFLAGS_OF_BIT)
#define X86_EFLAGS_IOPL_BIT	12 /* I/O Privilege Level (2 bits) */
#define X86_EFLAGS_IOPL		(3 << X86_EFLAGS_IOPL_BIT)
#define X86_EFLAGS_NT_BIT	14 /* Nested Task */
#define X86_EFLAGS_NT		_BITUL(X86_EFLAGS_NT_BIT)
#define X86_EFLAGS_RF_BIT	16 /* Resume Flag */
#define X86_EFLAGS_RF		_BITUL(X86_EFLAGS_RF_BIT)
#define X86_EFLAGS_VM_BIT	17 /* Virtual Mode */
#define X86_EFLAGS_VM		_BITUL(X86_EFLAGS_VM_BIT)
#define X86_EFLAGS_AC_BIT	18 /* Alignment Check/Access Control */
#define X86_EFLAGS_AC		_BITUL(X86_EFLAGS_AC_BIT)
#define X86_EFLAGS_VIF_BIT	19 /* Virtual Interrupt Flag */
#define X86_EFLAGS_VIF		_BITUL(X86_EFLAGS_VIF_BIT)
#define X86_EFLAGS_VIP_BIT	20 /* Virtual Interrupt Pending */
#define X86_EFLAGS_VIP		_BITUL(X86_EFLAGS_VIP_BIT)
#define X86_EFLAGS_ID_BIT	21 /* CPUID detection */
#define X86_EFLAGS_ID		_BITUL(X86_EFLAGS_ID_BIT)


// from linux: https://github.com/torvalds/linux/blob/ead751507de86d90fa250431e9990a8b881f713c/arch/x86/include/asm/sighandling.h
#define INITIAL_EFLAGS	(X86_EFLAGS_IOPL | X86_EFLAGS_FIXED | X86_EFLAGS_IF | X86_EFLAGS_ID)

// register save by the hardware context switch: SS, rsp, rip (pointer), rflags
#define hal_save_task_state() \
    pushq %rdi; \
    pushq %rsi; \
    pushq %rdx; \
    pushq %rcx; \
    pushq %rax; \
    pushq %r8;  \
    pushq %r9;  \
    pushq %r10; \
    pushq %r11; \
    pushq %rbx; \
    pushq %rbp; \
    pushq %r12; \
    pushq %r13; \
    pushq %r14; \
    pushq %r15;


#ifndef __ASSEMBLER__
#include <stdint.h>
#include <core/hal/native_task.h>

// native_task_t is hardware dependent
// IMPORTANT: Never changes the layout of native_task_t without change it in native_task.S

// from: https://github.com/torvalds/linux/blob/ead751507de86d90fa250431e9990a8b881f713c/arch/x86/include/uapi/asm/ptrace.h
typedef struct {
    /*
    * C ABI says these regs are callee-preserved. They aren't saved on kernel entry
    * unless syscall needs a complete, fully filled "struct pt_regs".
    */
        uint64_t r15;   // 0
        uint64_t r14;   // 1
        uint64_t r13;   // 2
        uint64_t r12;   // 3
        uint64_t rbp;   // 4
        uint64_t rbx;   // 5
    /* These regs are callee-clobbered. Always saved on kernel entry. */
        uint64_t r11;   // 6
        uint64_t r10;   // 7
        uint64_t r9;    // 8
        uint64_t r8;    // 9
        uint64_t rax;   // 10
        uint64_t rcx;   // 11
        uint64_t rdx;   // 12
        uint64_t rsi;   // 13
        uint64_t rdi;   // 14
    /*
    * On syscall entry, this is syscall#. On CPU exception, this is error code.
    * On hw interrupt, it's IRQ number:
    */
        uint64_t orig_rax;  // 15
    /* Return frame for iretq */
        uint64_t codeptr;       // 16
        uint64_t cs;            // 17
        uint64_t eflags;        // 18
        uint64_t stackptr;      // 19
        uint64_t ss;            // 20
    /* top of stack page */
} __attribute__((packed)) native_task_t;

/**
 * Given a pointer to a zero-filled native_task_t structure, fill the platform dependent information, apart
 * from the code pointer (entry function) and the stack ponter.
 * permission_mode: 
 *      0 -> user
 *      1 -> kernel
 */

native_task_t* hal_update_current_state(native_task_t *native_task_on_stack);

void intel_switch_task(native_task_t *task) __attribute__ ((noreturn));

#endif // __ASSEMBLER__

#endif