#ifndef _HAL_DEBUGGER_H
#define _HAL_DEBUGGER_H

#ifdef __ASSEMBLER__
// Supports both gdb (with rbreak) and bochs magic breakpoint
.macro MDEBUGGER name
.type  kernel_debug_asm_\name\@,%function
.globl kernel_debug_asm_\name\@
    kernel_debug_asm_\name\@: xchg %bx, %bx
.endm
#define DEBUGGER(name)        MDEBUGGER name

#else
#define DEBUGGER_LABEL(y)               ".kernel_debug_c_" #y
#define DEBUGGER_AUTO_LABEL(y)          DEBUGGER_LABEL(y)
#define DEBUGGER()                      ({ asm(DEBUGGER_AUTO_LABEL(__LINE__) ": xchg %bx, %bx;"); })
#endif

#endif