#include <logging.h>
#include <core/syscall.h>
#include <core/task_management.h>
#include <core/scheduler/services.h>

long syscall_log(int level, char* msg)
{
    task_t* task = scheduler_current_task();
    logging(level, "[%s]: %s", task_display_name(task), msg);
    return 0;
}

long syscall_exit()
{
    task_destroy(scheduler_current_task());
    // there is no return from this system call
    scheduler_switch_task();
    return 0; // never happens
}

long do_syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5)
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

    // TODO change to a table
    switch (syscall_number)
    {
        case 1:
            return syscall_exit();

        case 2:
            return syscall_log(arg1, (char*) arg2);
    }
    log_error("Invalid system call %d (0x%x) called from %s", syscall_number, syscall_number, task_display_name(scheduler_current_task()));
    return -1;
}