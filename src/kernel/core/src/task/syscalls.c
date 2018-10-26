#include <core/task/services.h>
#include <core/scheduler/services.h>

long syscall_handler_task_exit()
{
    task_t* task = scheduler_current_task();
    task_destroy(task);

    // there is no return from this syscall since the task was killed
    scheduler_switch_task();
    return 0;
}
