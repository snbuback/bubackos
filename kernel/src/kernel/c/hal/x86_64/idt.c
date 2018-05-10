#include <stdint.h>
#include <string.h>
#include <hal/gdt.h>
#include <hal/idt.h>
#include <core/logging.h>
#include <core/task_management.h>

/** Declare an IDT of 256 entries.  Although we will only use the
 * first 32 entries in this tutorial, the rest exists as a bit
 * of a trap.  If any undefined IDT entry is hit, it normally
 * will caused an "Unhandled Interrupt" exception.  Any descriptor
 * for which the 'presence' bit is cleared (0) will generate an
 * "Unhandled Interrupt" exception */
static volatile idt_entry idt[IDT_TOTAL_INTERRUPTIONS] = {};

/* Use this function to set an entry in the IDT.  A lot simpler
 * than twiddling with the GDT ;) */
void idt_set_gate(unsigned num, uintptr_t base, unsigned type, unsigned ring)
{
    log_trace("Set interrupt gate %d (0x%x) at %p type %x", num, num, base, type);
    idt[num].base_0_15 = (base & 0xFFFF);
    idt[num].base_16_31 = (base >> 16) & 0xFFFF;
    idt[num].base_32_63 = (base >> 32) & 0xFFFFFFFF;

    /* Finally, set up the granularity and access flags */
    idt[num].segment = GDT_SEGMENT(GDT_ENTRY_KERNEL_CS);
    idt[num].ist = 0; // 64 bits
    idt[num].type = type;
    idt[num].ring = ring;
    idt[num].present = 1;
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
    uint8_t key;
    task_id_t task_id;

    switch (interrupt) {
    case 0x8: // timer
        // log_debug("timer");
        // ack int (PIC_MASTER_CMD, PIC_CMD_EOI)
        outb(0x20, 0x20);
        do_task_switch();
        break;
    case 0x9: // keyboard
        key = inb(0x60);
        log_trace("Key pressed: 0x%x", key);
        break;
    case 0xD: // GP
        task_id = get_current_task();
        if (task_id != NULL_TASK) {
            log_fatal("General protection caused by task %d. Killing task.", task_id);
            task_destroy(task_id);
        } else {
            log_fatal("General protection caused by kernel :,(");
        }
        do_task_switch();
        break;
    default:
        log_info("Interruption %d (0x%x) / (%d) 0x%x generated", interrupt, interrupt, param, param);
        break;
    }
    return;
}

/* Installs the IDT */
void idt_install()
{
    log_debug("IDT Table at %p of size %d bytes", &idt, sizeof(idt));

    idt_fill_table();

    idt_flush((uintptr_t) &idt, sizeof(idt) - 1);
}
