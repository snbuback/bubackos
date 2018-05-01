#ifndef _HAL_NATIVE_TASK_H
#define _HAL_NATIVE_TASK_H

#ifndef ASM_FILE
#include <stdint.h>
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

// assembly
void hal_switch_task(native_task_t *task)  __attribute__ ((noreturn));

/**
 * Given a pointer to a zero-filled native_task_t structure, fill the platform dependent information, apart
 * from the code pointer (entry function) and the stack ponter.
 */
void hal_create_native_task(native_task_t *task, uintptr_t code, uintptr_t stack);

#endif // ASM_FILE

//21
#define HAL_NATIVE_TASK_SIZE    21*8 // sizeof(native_task_t)


#endif