#include <hal/configuration.h>
#include <x86_64/gdt.h>
#include <x86_64/native_task.h>
#include <logging.h>
#include <core/task/services.h>
#include <core/scheduler/services.h>
#include <core/hal/native_task.h>

bool native_task_create(task_t* task, uintptr_t code, uintptr_t stack, int permission_mode, uintptr_t userdata)
{
    // TODO FIX native_task size
    native_task_t* ntask = (native_task_t*) kalloc(512);
    if (!ntask) {
        return false;
    }
    ntask->rbp = stack;
    ntask->stackptr = stack;
    ntask->codeptr = code;
    ntask->eflags = INITIAL_EFLAGS;

    if (permission_mode) {
        // kernel module
        ntask->cs = (GDT_SEGMENT(GDT_ENTRY_KERNEL_CS)+GDT_RING_SYSTEM);
        ntask->ss = (GDT_SEGMENT(GDT_ENTRY_KERNEL_DS)+GDT_RING_SYSTEM);
    } else {
        ntask->cs = (GDT_SEGMENT(GDT_ENTRY_KERNEL_CS)+GDT_RING_USER);
        ntask->ss = (GDT_SEGMENT(GDT_ENTRY_KERNEL_DS)+GDT_RING_USER);
    }

    ntask->rdi = userdata;
    task->native_task = ntask;
    return true;
}

void intel_switch_task(native_task_t *task) __attribute__ ((noreturn));

__attribute__ ((noreturn)) void native_task_switch(task_t* task)
{
    if (task) {
        native_task_t* ntask = (native_task_t*) task->native_task;
        log_trace("Switching to task %s [%d]. Code at %p, stack at %p",
            task_display_name(task),
            task->task_id,
            ntask->codeptr,
            ntask->stackptr);

        intel_switch_task(ntask);
    } else {
        native_task_sleep();
    }
}

native_task_t* hal_update_current_state(native_task_t *native_task_on_stack)
{
    task_t* task = scheduler_current_task();
    if (!task) {
        // no status to update
        log_error("Invalid task_update_current_state called");
        // to avoid error in syscall_jumper
        return native_task_on_stack;
    }
    native_task_t* native_task = (native_task_t*) task->native_task;

    // sysenter requires a update to the segment registers
    native_task_on_stack->cs = native_task->cs;
    native_task_on_stack->ss = native_task->ss;

    // copy content from native_task
    *native_task = *native_task_on_stack;
    return native_task;
}

