#ifndef __GDT_H
#define __GDT_H
#include <stdint.h>

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define GDT_SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define GDT_SEG_PRES(x)      ((x) << 0x07) // Present
#define GDT_SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define GDT_SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define GDT_SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define GDT_SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define GDT_SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)

#define GDT_SEG_DATA_RD        0x00 // Read-Only
#define GDT_SEG_DATA_RDA       0x01 // Read-Only, accessed
#define GDT_SEG_DATA_RDWR      0x02 // Read/Write
#define GDT_SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define GDT_SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define GDT_SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define GDT_SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define GDT_SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define GDT_SEG_CODE_EX        0x08 // Execute-Only
#define GDT_SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define GDT_SEG_CODE_EXRD      0x0A // Execute/Read
#define GDT_SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define GDT_SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define GDT_SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define GDT_SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define GDT_SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed

extern void _gdt_flush();
extern void gdt_set_gate(uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
extern void gdt_install();

#endif
