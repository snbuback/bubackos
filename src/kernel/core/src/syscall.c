#include <logging.h>
#include <core/syscall.h>
#include <core/task_management.h>

long syscall_log(int level, char* msg)
{
    task_id_t task_id = get_current_task();
    logging(level, "task %d: %s", task_id, msg);
    return 0;
}

long syscall_exit()
{
    return task_destroy(get_current_task());
}

long do_syscall(long syscall_number, long arg2, long arg3, long arg4, long arg5)
{
    log_trace("Syscall %x called from %d: arg2=%x arg3=%x arg4=%x arg5=%x", 
        syscall_number,
        get_current_task(),
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
            return syscall_log(arg2, (char*) arg3);
    }
    log_error("Invalid system call %d called from %d", syscall_number, get_current_task());
    return 0;
}