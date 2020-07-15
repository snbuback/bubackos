#include <stdint.h>
#include <logging.h>
#include <x86_64/gdt.h>
#include <x86_64/idt.h>
#include <x86_64/intel.h>
#include <core/scheduler/services.h>
#include <core/hal/hw_events.h>

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

__attribute((noreturn)) static inline void handle_task_switch(native_task_t *native_task)
{
    // ack int (PIC_MASTER_CMD, PIC_CMD_EOI)
    outb(0x20, 0x20);
    if (scheduler_current_task()) {
        hal_update_current_state(native_task);
    }
    scheduler_switch_task();
}

native_task_t* pre_syscall(native_task_t* native_task)
{
    native_task->rax = handle_syscall(
        native_task->rdi,
        native_task->rsi,
        native_task->rdx,
        native_task->r10,
        native_task->r8,
        native_task->r9
    );
    // the returning is important to the syscall_jumper
    return native_task;
}

void parse_intel_pagefault_flag(pagefault_status_t* pf, unsigned int pf_flag)
{
    pf->is_reference_valid = !(pf_flag & PF_FLAG_PRESENT);
    pf->no_execution_access = pf_flag & PF_FLAG_INSTRUCTION;
    pf->no_reading_access = !(pf_flag & PF_FLAG_READ_WRITE);
    pf->no_writing_access = pf_flag & PF_FLAG_READ_WRITE;
}

void interrupt_handler(native_task_t *native_task, int interrupt)
{
    log_debug("Interruption %d (0x%x) on task %s. kernel-stack=%p",
        interrupt,
        interrupt,
        task_display_name(scheduler_current_task()),
        get_stack_base_addr()
        );
    if (interrupt != 0x8) {
        DEBUGGER;
    }

    switch (interrupt) {
    case 0x8: // timer
        handle_task_switch(native_task);
        break;

    case 0xD: // GP
        handle_protection_fault();
        break;

    case 0xE: {  // Page fault
        pagefault_status_t pf = {
            .addr = page_fault_addr(),
            .codeptr = native_task->codeptr,
            .stackptr = native_task->stackptr

        };
        parse_intel_pagefault_flag(&pf, native_task->orig_rax);
        handle_page_fault(pf);
        break;
    }

    case 0x6: // Invalid OPCODE
        handle_invalid_operation();
        break;

    case INT_SYSTEM_CALL:
        pre_syscall(native_task);
        // in case the syscall returns, the caller task will still executes.
        intel_switch_task(native_task);
        break;

    default:
        hal_update_current_state(native_task);
        log_info("Unmapped interruption %d (0x%x) with param %d called", interrupt, interrupt, native_task->orig_rax);
        handle_generic_hw_events();
        break;
    }

    scheduler_switch_task();
}

void idt_initialize()
{
    idt_fill_table();
    idt_install();
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

