#ifndef __GDT_H
#define __GDT_H
#include <stdint.h>
#include <stdbool.h>

/* Intel manual vol 3A, 3-13 */

// GDT Type
#define GDT_TYPE_SYSTEM                0x00
#define GDT_TYPE_CODE_OR_DATA          0x01


// GDT Type System and Gates
#define GDT_TYPE_LDT                    0x02
#define GDT_TYPE_TSS_AVAILABLE          0x09
#define GDT_TYPE_TSS_BUSY               0x11
#define GDT_TYPE_CALL_GATE              0x12
#define GDT_TYPE_INTERRUPT_GATE         0x14
#define GDT_TYPE_TRAP_GATE              0x15

// GDT Type Code and Data
#define GDT_TYPE_SEG_DATA_RD        0x10 // Read-Only
#define GDT_TYPE_SEG_DATA_RDA       0x11 // Read-Only, accessed
#define GDT_TYPE_SEG_DATA_RDWR      0x12 // Read/Write
#define GDT_TYPE_SEG_DATA_RDWRA     0x13 // Read/Write, accessed
#define GDT_TYPE_SEG_DATA_RDEXPD    0x14 // Read-Only, expand-down
#define GDT_TYPE_SEG_DATA_RDEXPDA   0x15 // Read-Only, expand-down, accessed
#define GDT_TYPE_SEG_DATA_RDWREXPD  0x16 // Read/Write, expand-down
#define GDT_TYPE_SEG_DATA_RDWREXPDA 0x17 // Read/Write, expand-down, accessed
#define GDT_TYPE_SEG_CODE_EX        0x18 // Execute-Only
#define GDT_TYPE_SEG_CODE_EXA       0x19 // Execute-Only, accessed
#define GDT_TYPE_SEG_CODE_EXRD      0x1A // Execute/Read
#define GDT_TYPE_SEG_CODE_EXRDA     0x1B // Execute/Read, accessed
#define GDT_TYPE_SEG_CODE_EXC       0x1C // Execute-Only, conforming
#define GDT_TYPE_SEG_CODE_EXCA      0x1D // Execute-Only, conforming, accessed
#define GDT_TYPE_SEG_CODE_EXRDC     0x1E // Execute/Read, conforming
#define GDT_TYPE_SEG_CODE_EXRDCA    0x1F // Execute/Read, conforming, accessed

// Privilege level
#define GDT_RING_SYSTEM                 0   // kernel ring
#define GDT_RING_USER                   3   // user ring

#define GDT_MAXIMUM_SIZE                  100
#define GDT_MAXIMUM_MEMORY                0xFFFFFFFF

void gdt_install();

#endif
