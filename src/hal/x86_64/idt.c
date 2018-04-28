#include <stdint.h>
#include <string.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/logging.h>

typedef enum
{
	TASK_GATE_286 = 0x5,
	INTERRUPT_GATE_286 = 0x6,
	TRAP_GATE_286 = 0x7,
	INTERRUPT_GATE_386 = 0xE,
	TRAP_GATE_386 = 0xF
} enum_gate_type;


/* Defines an IDT entry */
typedef struct {
    unsigned base_0_15 : 16;
    unsigned segment : 16;
    unsigned ist : 3;
    unsigned : 5;
    enum_gate_type type : 4;
    unsigned : 1;
    unsigned ring : 2;
    unsigned present : 1;
    unsigned base_16_31 : 16;
    unsigned base_32_63 : 32;
    unsigned : 32;
} __attribute__((packed)) idt_entry;

typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr;

/** Declare an IDT of 256 entries.  Although we will only use the
 * first 32 entries in this tutorial, the rest exists as a bit
 * of a trap.  If any undefined IDT entry is hit, it normally
 * will caused an "Unhandled Interrupt" exception.  Any descriptor
 * for which the 'presence' bit is cleared (0) will generate an
 * "Unhandled Interrupt" exception */
static idt_entry idt[256];

/* Use this function to set an entry in the IDT.  A lot simpler
 * than twiddling with the GDT ;) */
void idt_set_gate(unsigned char num, uintptr_t base, enum_gate_type type)
{
    // log_debug("Set interrupt gate %d (0x%x) at %p type %x", num, num, base, type);
    idt[num].base_0_15 = (base & 0xFFFF);
    idt[num].base_16_31 = (base >> 16) & 0xFFFF;
    idt[num].base_32_63 = (base >> 32) & 0xFFFFFFFF;

    /* Finally, set up the granularity and access flags */
    idt[num].segment = GDT_SEGMENT(GDT_ENTRY_KERNEL_CS);
    idt[num].ist = 0; // 64 bits
    idt[num].type = type;
    idt[num].ring = 0;
    idt[num].present = 1;
}

bool are_interrupts_enabled()
{
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}

void idt_flush()
{
    idt_ptr idt_address;
    /* Setup the GDT pointer and limit */
    idt_address.limit = sizeof(idt) - 1;
    idt_address.base = (uint64_t)&idt;

    log_debug("Flushing IDT table at %p size of %d bytes (0x%x)", idt_address.base, idt_address.limit, idt_address.limit);

    __asm__("lidt %0"
            :
            : "m"(idt_address));
}

struct interrupt_frame {
    uintptr_t ip;
    uintptr_t cs;
    uintptr_t flags;
    uintptr_t sp;
    uintptr_t ss;
};

void int_doublefault_handler() {
    log_info("Double fault");
}

void int_generalfault_handler() {
    log_info("General fault");
    asm volatile ("cli; hlt;");
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

void interrupt_handler(uint64_t interrupt, uint64_t param)
{
    if (interrupt == 8) {
        // timer
        outb(0x70, inb(0x70)&0x7F);
    } else {
        log_info("Interruption %d (0x%x) / (%d) 0x%x generated", interrupt, interrupt, param, param);
    }
    return;
}

/* Installs the IDT */
void idt_install()
{
    /* Clear out the entire IDT, initalizing it to zeros */
    memset(idt, 0, sizeof(idt));

    extern uintptr_t intr0; idt_set_gate(0, (uintptr_t)&intr0, INTERRUPT_GATE_386);
    extern uintptr_t intr1; idt_set_gate(1, (uintptr_t)&intr1, INTERRUPT_GATE_386);
    extern uintptr_t intr2; idt_set_gate(2, (uintptr_t)&intr2, INTERRUPT_GATE_386);
    extern uintptr_t intr3; idt_set_gate(3, (uintptr_t)&intr3, INTERRUPT_GATE_386);
    extern uintptr_t intr4; idt_set_gate(4, (uintptr_t)&intr4, INTERRUPT_GATE_386);
    extern uintptr_t intr5; idt_set_gate(5, (uintptr_t)&intr5, INTERRUPT_GATE_386);
    extern uintptr_t intr6; idt_set_gate(6, (uintptr_t)&intr6, INTERRUPT_GATE_386);
    extern uintptr_t intr7; idt_set_gate(7, (uintptr_t)&intr7, INTERRUPT_GATE_386);
    extern uintptr_t intr8; idt_set_gate(8, (uintptr_t)&intr8, INTERRUPT_GATE_386);
    extern uintptr_t intr9; idt_set_gate(9, (uintptr_t)&intr9, INTERRUPT_GATE_386);
    extern uintptr_t intr10; idt_set_gate(10, (uintptr_t)&intr10, INTERRUPT_GATE_386);
    extern uintptr_t intr11; idt_set_gate(11, (uintptr_t)&intr11, INTERRUPT_GATE_386);
    extern uintptr_t intr12; idt_set_gate(12, (uintptr_t)&intr12, INTERRUPT_GATE_386);
    extern uintptr_t intr13; idt_set_gate(13, (uintptr_t)&intr13, INTERRUPT_GATE_386);
    extern uintptr_t intr14; idt_set_gate(14, (uintptr_t)&intr14, INTERRUPT_GATE_386);
    extern uintptr_t intr15; idt_set_gate(15, (uintptr_t)&intr15, INTERRUPT_GATE_386);
    extern uintptr_t intr16; idt_set_gate(16, (uintptr_t)&intr16, INTERRUPT_GATE_386);
    extern uintptr_t intr17; idt_set_gate(17, (uintptr_t)&intr17, INTERRUPT_GATE_386);
    extern uintptr_t intr18; idt_set_gate(18, (uintptr_t)&intr18, INTERRUPT_GATE_386);

    idt_flush();
}

void hi() {
    log_info("hi");
}