#include <logging.h>
#include <core/hal/hw_events.h>
#include <core/scheduler/services.h>
#include <core/syscall/services.h>

static syscall_handler_func syscall_vector[SYSCALL_MAX_NUMBER];

// fill syscall vector
bool syscall_initialize() {
    return syscall_registering();
}

bool register_syscall(unsigned long syscall_number, syscall_handler_func handler)
{
    if (syscall_number == 0 || syscall_number >= SYSCALL_MAX_NUMBER) {
        log_error("Error registering syscall %d and handler %p. Number out of range", syscall_number, handler);
        return false;
    }

    if (syscall_vector[syscall_number]) {
        log_error("Error registering syscall %d and handler %p. Already registered", syscall_number, handler);
        return false;
    }

    syscall_vector[syscall_number] = handler;
    return true;
}

long handle_syscall(unsigned long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5)
{
    log_trace("Syscall %x called from %s: arg1=0x%x arg2=0x%x arg3=0x%x arg4=0x%x arg5=0x%x", 
        syscall_number,
        task_display_name(scheduler_current_task()),
        arg1,
        arg2,
        arg3,
        arg4,
        arg5
    );

    if (syscall_number == 0 || syscall_number >= SYSCALL_MAX_NUMBER) {
        log_error("Invalid syscall %d from task %s. Number out of range.",
            syscall_number,
            task_display_name(scheduler_current_task())
        );
        return -1;
    }

    syscall_handler_func handler = syscall_vector[syscall_number];
    if (!handler) {
        log_error("Invalid syscall %d from task %s. Unregistered handler.",
            syscall_number,
            task_display_name(scheduler_current_task())
        );
        return -1;
    }

    return handler(arg1, arg2, arg3, arg4, arg5);
}

