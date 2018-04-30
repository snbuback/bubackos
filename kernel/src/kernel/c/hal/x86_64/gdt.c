#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <core/configuration.h>
#include <core/logging.h>
#include <hal/gdt.h>
#include <hal/tss.h>

#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

gdt_entry gdt[GDT_MAXIMUM_SIZE];
tss_entry_t tss_entry;

/* Setup a descriptor in the Global Descriptor Table */
static uint16_t gdt_set_gate(uint16_t num, uint64_t base, uint32_t limit, uint8_t type, uint8_t ring)
{
    // log_debug("Installing GDT %d at %p sizeof %x type %x ring %d", num, base, limit, type, ring);
    // this code only works for 64 bits GDT. Also granulatiry if always 1, that means limit are multiples of 4kb
    limit = limit >> 12;

    gdt[num].base_0_15 = (base & 0xFFFF);
    gdt[num].base_16_23 = (base >> 16) & 0xFF;
    gdt[num].base_24_31 = (base >> 24) & 0xFF;
    gdt[num].base_32_63 = (base >> 32) & 0xFFFFFFFF;

    /* Setup the descriptor limits */
    gdt[num].limit_0_15 = (limit & 0xFFFF);
    gdt[num].limit_16_19 = (limit >> 16) & 0x0F;

    /* Finally, set up the granularity and access flags */
    gdt[num].available = 1;
    gdt[num].longmode = 1; // 64 bits
    gdt[num].op_size = 0; // 64 bits (in the manual said should set as 16 bit when in longmode)
    gdt[num].granularity = 1; // limit is 4kb multiples
    gdt[num].type = type;
    gdt[num].ring = ring;
    gdt[num].present = 1;
    return num;
}

extern uintptr_t stack_end;
static void tss_set(tss_entry_t *tss) {
    log_info("TSS installed at %p size 0x%x", &tss, sizeof *tss);
    tss->rsp0 = (uint64_t) malloc(SYSTEM_STACKSIZE);
    tss->rsp1 = (uint64_t) malloc(SYSTEM_STACKSIZE);
    tss->rsp2 = (uint64_t) malloc(SYSTEM_STACKSIZE);
}

void gdt_install()
{
    /* Clear GDT table. Also insert the NULL GDT */
    memset(gdt, 0, sizeof(gdt));

    gdt_set_gate(GDT_ENTRY_KERNEL_CS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_CODE_EXRD, GDT_RING_SYSTEM);
    gdt_set_gate(GDT_ENTRY_KERNEL_DS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_DATA_RDWR, GDT_RING_SYSTEM);
    gdt_set_gate(GDT_ENTRY_USER_CS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_CODE_EXRD, GDT_RING_USER);
    gdt_set_gate(GDT_ENTRY_USER_DS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_DATA_RDWR, GDT_RING_USER);
    gdt_set_gate(GDT_ENTRY_TSS, (uint64_t) &tss_entry, sizeof(tss_entry), GDT_TYPE_TSS_AVAILABLE, GDT_RING_SYSTEM);
    tss_set(&tss_entry);

    /* Flush our the old GDT / TSS and install the new changes! */
    uint16_t gdt_limit = (sizeof(gdt_entry) * GDT_MAXIMUM_SIZE) - 1;
	// log_debug("Flushing GDT table at %p, size of %x", &gdt, gdt_limit);
    gdt_flush((uintptr_t) &gdt, gdt_limit);

	// log_debug("Flushing TSS table using entry %d", GDT_ENTRY_TSS);
    tss_flush(GDT_SEGMENT(GDT_ENTRY_TSS));
}
