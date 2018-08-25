#include <hal/configuration.h>
#include <x86_64/gdt.h>
#include <x86_64/native_task.h>
#include <logging.h>

void hal_create_native_task(native_task_t *task, uintptr_t code, uintptr_t stack, int permission_mode, uintptr_t userdata)
{
    task->rbp = stack;
    task->stackptr = stack;
    task->codeptr = code;
    task->eflags = INITIAL_EFLAGS;

    if (permission_mode) {
        // kernel module
        task->cs = (GDT_SEGMENT(GDT_ENTRY_KERNEL_CS)+GDT_RING_SYSTEM);
        task->ss = (GDT_SEGMENT(GDT_ENTRY_KERNEL_DS)+GDT_RING_SYSTEM);
    } else {
        task->cs = (GDT_SEGMENT(GDT_ENTRY_KERNEL_CS)+GDT_RING_USER);
        task->ss = (GDT_SEGMENT(GDT_ENTRY_KERNEL_DS)+GDT_RING_USER);
    }

    task->rdi = userdata;
}
