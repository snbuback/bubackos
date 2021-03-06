#ifndef _HAL_DEBUGGER_H
#define _HAL_DEBUGGER_H

// Supports both gdb (with rbreak) and bochs magic breakpoint
#ifdef __ASSEMBLER__
.macro DEBUGGER
    xchg %bx, %bx
.endm

#else  // C

#define DEBUGGER                      asm volatile ("xchg %bx, %bx")
#endif

#endif