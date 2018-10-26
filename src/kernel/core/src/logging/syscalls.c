#include <logging.h>
#include <core/scheduler/services.h>

long syscall_handler_logging(int level, char* msg)
{
    task_t* task = scheduler_current_task();
    logging(level, "[%s]: %s", task_display_name(task), msg);
    return 0;
}


