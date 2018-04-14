#include <kernel/logging.h>
#include <kernel/gdt.h>
#include <stdbool.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

/* Defines a GDT entry.  We say packed, because it prevents the
 * compiler from doing things that it thinks is best, i.e.
 * optimization, etc. */

typedef struct {
    unsigned limit_0_15 : 16;
    unsigned base_0_15 : 16;
    unsigned base_16_23 : 8;
    unsigned type : 5; // includes the type and the next bit and is 1 only for code/data segments
    unsigned ring : 2;
    unsigned present : 1;
    unsigned limit_16_19 : 4;
    unsigned available : 1;
    unsigned longmode : 1;
    unsigned op_size : 1;
    unsigned granularity : 1;
    unsigned base_24_31 : 8;
    unsigned base_32_63 : 32;
    unsigned : 32;
} __attribute__((packed)) gdt_entry;

/* Special pointer which includes the limit: The max bytes taken up by the GDT, minus 1.*/
typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr;

gdt_entry gdt[GDT_MAXIMUM_SIZE];
volatile uint16_t last_id = 1;

/* Setup a descriptor in the Global Descriptor Table */
uint16_t gdt_set_gate(uint64_t base, uint32_t limit, uint8_t type, uint8_t ring)
{
    // this code only works for 64 bits GDT. Also granulatiry if always 1, that means limit are multiples of 4kb
    limit = limit >> 12;

    // next available entry
    uint16_t num = last_id++;

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

/* Setup the GDT pointer and limit */
void gdt_flush(void)
{
    gdt_ptr gdt_address;
    gdt_address.limit = (sizeof(gdt_entry) * last_id) - 1;
    gdt_address.base = (uint64_t)&gdt;
	log_debug("Flushing GDT table at %p, size of %d", gdt_address.base, gdt_address.limit);
	asm volatile(
        "lgdt %0"
        :   // output operands
        : "m" (gdt_address) //input operands
    );
}

/** Should be called by main.  This will setup the special GDT
 * pointer, set up the 6 entries in our GDT, and then finally
 * call gdt_flush() in our assembler file in order to tell
 * the processor where the new GDT is and update the new segment
 * registers. */
void gdt_install()
{
    /* Clear GDT table. Also insert the NULL GDT */
    memset(gdt, 0, sizeof(gdt));

    gdt_set_gate(0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_CODE_EXRD, GDT_RING_SYSTEM);
    gdt_set_gate(0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_DATA_RDWR, GDT_RING_SYSTEM);
    gdt_set_gate(0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_CODE_EXRD, GDT_RING_USER);
    gdt_set_gate(0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_DATA_RDWR, GDT_RING_USER);

    /* Install the TSS into the GDT */
    //tss_install(5, 0x10, 0x0);

    /* Flush our the old GDT / TSS and install the new changes! */
    gdt_flush();
    //_tss_flush();
}
