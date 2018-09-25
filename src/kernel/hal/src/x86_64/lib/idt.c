#include <stdint.h>
#include <string.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <logging.h>
#include <core/scheduler/services.h>
#include <core/syscall.h>
#include <x86_64/intel.h>
#include <core/scheduler/services.h>
#include <core/hal/native_vmem.h>

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
    // log_trace("Set interrupt gate %d (0x%x) at %p type %x", num, num, base, type);
    idt[num].base_0_15 = (base & 0xFFFF);
    idt[num].base_16_31 = (base >> 16) & 0xFFFF;
    idt[num].base_32_63 = (base >> 32) & 0xFFFFFFFF;

    /* Finally, set up the granularity and access flags */
    idt[num].segment = GDT_SEGMENT(GDT_ENTRY_KERNEL_CS);
    idt[num].ist = 1; // 64 bits
    // Important: In the TSS (GDT) we are loading the kernel stack on IST1 (improve this code)
    idt[num].type = type;
    idt[num].ring = ring;
    idt[num].present = 1;
}

void handle_keyboard() {
    outb(0x20, 0x20);
    // uint8_t key = inb(0x60);
    // log_trace("Key pressed: 0x%x", (unsigned int) key);
}

void handle_general_protection(native_task_t *native_task)
{
    task_t* task = scheduler_current_task();
    log_fatal("GP: code=%p stack=%p task=%s", native_task->codeptr, native_task->stackptr, task_display_name(task));
    if (task) {
        log_fatal("General protection caused by task %s. Killing task.", task_display_name(task));
        task_destroy(task);
    } else {
        log_fatal("General protection caused by kernel :,(");
    }
    scheduler_switch_task();
}

// TODO move CR2 interpreter to native method
void handle_general_page_fault(native_task_t *native_task)
{
    uintptr_t memory;
    asm volatile( "mov %%cr2, %0"
                   : "=r" (memory));

    task_t* task = scheduler_current_task();
    log_fatal("PF: addr=%p cd=%p st=%p fl=%x tk=%s", memory, native_task->codeptr, native_task->stackptr, native_task->orig_rax, task_display_name(task));

    if (task) {
        log_fatal("Page fault caused by task %s. Killing task.", task_display_name(task));
        task_destroy(task);
    } else {
        log_fatal("Page fault caused by kernel :,(");
    }
    native_vmem_dump(NULL);
    scheduler_switch_task();
}

__attribute((noreturn)) void handle_task_switch(native_task_t *native_task)
{
    // ack int (PIC_MASTER_CMD, PIC_CMD_EOI)
    outb(0x20, 0x20);
    if (scheduler_current_task()) {
        hal_update_current_state(native_task);
    }
    scheduler_switch_task();
}

static uintptr_t get_stack_addr()
{
    uintptr_t addr;
    asm volatile ("movq %%rbp, %0" : "=r"(addr));
    return addr;
}

__attribute((noreturn)) void pre_syscall(native_task_t* native_task)
{
    native_task->rax = do_syscall(
        native_task->rdi,
        native_task->rsi,
        native_task->rdx,
        native_task->r10,
        native_task->r8,
        native_task->r9
        );
    handle_task_switch(native_task);
}

void interrupt_handler(native_task_t *native_task, int interrupt)
{
    log_debug("Interruption %d (0x%x) on task %s and stack=%p", interrupt, interrupt, task_display_name(scheduler_current_task()), get_stack_addr());

    switch (interrupt) {
    case 0x8: // timer
        handle_task_switch(native_task);
        break;

    case 0x9: // keyboard
        handle_keyboard();
        break;

    case 0xD: // GP
        handle_general_protection(native_task);
        break;

    case 0xE: // Page fault
        handle_general_page_fault(native_task);
        break;

    case 0x6: // Invalid OPCODE
        log_info("Invalid OPCODE");
        handle_general_protection(native_task);
        break;

    case INT_SYSTEM_CALL:
        pre_syscall(native_task);
        break;

    default:
        hal_update_current_state(native_task);
        log_info("Unmapped interruption %d (0x%x) with param %d called", interrupt, interrupt, native_task->orig_rax);
        break;
    }

    scheduler_switch_task();
}

void idt_initialize()
{
    idt_fill_table();
}

/**
 * Intel manual Vol. 3A 5-23
 */
void syscall_install()
{
    // store entry pointer
    wrmsr(MSR_LSTAR, (uint64_t) &syscall_jumper);

    // store cs for user/kernel mode
    uint64_t user_cs = GDT_SEGMENT(GDT_ENTRY_USER_CS);
    uint64_t kernel_cs = GDT_SEGMENT(GDT_ENTRY_KERNEL_CS);
    uint64_t msr_star = ((user_cs << 16) | kernel_cs) << 32;
    wrmsr(MSR_STAR, msr_star);

    // flags to clear on syscall
    wrmsr(MSR_SYSCALL_MASK, X86_EFLAGS_TF|X86_EFLAGS_DF|X86_EFLAGS_IF|
	       X86_EFLAGS_IOPL|X86_EFLAGS_AC|X86_EFLAGS_NT);

    // kernel stack pointer
    wrmsr(MSR_KERNEL_GS_BASE, (uintptr_t) get_kernel_stack());

    // enable syscall/sysret
    uint64_t msr_efer = rdmsr(MSR_EFER);
    wrmsr(MSR_EFER, msr_efer | EFER_SCE);
}

/* Installs the IDT */
void idt_install()
{
    log_debug("IDT Table at %p of size %d bytes", &idt, sizeof(idt));
    idt_flush((uintptr_t) &idt, sizeof(idt) - 1);
    syscall_install(get_kernel_stack());
}
