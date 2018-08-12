#include <logging.h>
#include <core/syscall.h>
#include <core/task_management.h>

long do_syscall(native_task_t *native_task, long syscall_number)
{
    log_debug("Syscall %d called %p from %d", syscall_number, native_task, get_current_task());
    if (syscall_number == 1) {
        task_destroy(get_current_task());
        return 0;
    }
    return -1;
}