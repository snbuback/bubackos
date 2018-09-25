#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <core/hal/platform.h>
#include <core/alloc.h>
#include <logging.h>
#include <x86_64/gdt.h>
#include <libutils/utils.h>

static gdt_entry_t gdt[GDT_MAXIMUM_SIZE] __attribute__ ((aligned));
static tss_entry_t tss_entry __attribute__ ((aligned));
static uintptr_t* kernel_stacks __attribute__ ((aligned)); // per cpu

/* Setup a descriptor in the Global Descriptor Table */
uint16_t gdt_set_gate(uint16_t num, uint64_t base, uint32_t limit, uint8_t type, uint8_t ring)
{
    log_trace("Installing GDT %d at %p sizeof %x type %x ring %d", num, base, limit, type, ring);
    // this code only works for 64 bits GDT. Also granulatiry if always 1, that means limit are multiples of 4kb
    limit = limit >> 12;

    gdt_entry_t new_entry;

    new_entry.base_0_15 = (base & 0xFFFF);
    new_entry.base_16_23 = (base >> 16) & 0xFF;
    new_entry.base_24_31 = (base >> 24) & 0xFF;
    new_entry.base_32_63 = (base >> 32) & 0xFFFFFFFF;

    /* Setup the descriptor limits */
    new_entry.limit_0_15 = (limit & 0xFFFF);
    new_entry.limit_16_19 = (limit >> 16) & 0x0F;

    /* Finally, set up the granularity and access flags */
    new_entry.available = 1;
    new_entry.longmode = 1; // 64 bits
    new_entry.op_size = 0; // 64 bits (in the manual said should set as 16 bit when in longmode)
    new_entry.granularity = 1; // limit is 4kb multiples
    new_entry.type = type;
    new_entry.ring = ring;
    new_entry.present = 1;

    // GDT entry is created as local variable and assigned in the end of this function call. This helps
    // debug process that watches for writing in gdt memory addres.
    gdt[num] = new_entry;
    return num;
}

static void tss_set(tss_entry_t *tss) {
    uintptr_t kernel_stack_address = *get_kernel_stack();
    tss->rsp0_0_31 = kernel_stack_address & 0xFFFFFFFF;
    tss->rsp0_32_63 = kernel_stack_address >> 32;
    tss->ist1_0_31 = kernel_stack_address & 0xFFFFFFFF;
    tss->ist1_32_63 = kernel_stack_address >> 32;
    log_trace("TSS installed at %p size 0x%x (stack at %p)", &tss, sizeof *tss, kernel_stack_address);
    // tss->rsp1 = (uint64_t) (kalloc(SYSTEM_STACKSIZE) + SYSTEM_STACKSIZE);
    // tss->rsp2 = (uint64_t) (kalloc(SYSTEM_STACKSIZE) + SYSTEM_STACKSIZE);
}

uintptr_t* get_kernel_stack() {
    // as today there is support for 1 cpu
    return kernel_stacks;
}

void gdt_install()
{
    /* Clear GDT table. Also insert the NULL GDT */
    memset(gdt, 0, sizeof(gdt));
    memset(&tss_entry, 0, sizeof(tss_entry));

    // initialize kernel stack per cpu (currently 1)
    kernel_stacks = (uintptr_t*) kalloc(1 * sizeof(uintptr_t));
    kernel_stacks[0] = ALIGN((uintptr_t) (kalloc(SYSTEM_STACKSIZE) + SYSTEM_STACKSIZE), SYSTEM_PAGE_SIZE);
    log_trace("Kernel stack at %p (pointer to pointer at %p)", kernel_stacks[0], kernel_stacks);
    log_trace("*get_kernel_stack() = %p", *get_kernel_stack());

    gdt_set_gate(GDT_ENTRY_KERNEL_CS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_CODE_EXRD, GDT_RING_SYSTEM);
    gdt_set_gate(GDT_ENTRY_KERNEL_DS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_DATA_RDWR, GDT_RING_SYSTEM);
    gdt_set_gate(GDT_ENTRY_USER_CS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_CODE_EXRD, GDT_RING_USER);
    gdt_set_gate(GDT_ENTRY_USER_DS, 0x0, GDT_MAXIMUM_MEMORY, GDT_TYPE_SEG_DATA_RDWR, GDT_RING_USER);
    gdt_set_gate(GDT_ENTRY_TSS, (uint64_t) &tss_entry, sizeof(tss_entry), GDT_TYPE_TSS_AVAILABLE, GDT_RING_SYSTEM);
    tss_set(&tss_entry);

    /* Flush our the old GDT / TSS and install the new changes! */
    uint16_t gdt_limit = (sizeof(gdt_entry_t) * GDT_MAXIMUM_SIZE) - 1;
    log_trace("Flushing GDT table at %p, size of %x", &gdt, gdt_limit);
    gdt_flush((uintptr_t) &gdt, gdt_limit);

    log_trace("Flushing TSS table using entry %d (%x)", GDT_ENTRY_TSS, GDT_SEGMENT(GDT_ENTRY_TSS));
    tss_flush(GDT_SEGMENT(GDT_ENTRY_TSS));
}

__attribute((weak)) void gdt_flush(uintptr_t base, uint16_t limit)
{
    gdt_ref_t table_ref = {
        .base = base,
        .limit = limit
    };

    asm volatile ("lgdt (%0)" : : "r" (&table_ref));
}

__attribute((weak)) void tss_flush(uint16_t gdt_entry_number)
{
    asm volatile ("ltr %0": : "r"(gdt_entry_number));
}
