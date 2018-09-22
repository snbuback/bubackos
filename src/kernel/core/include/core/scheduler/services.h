#ifndef _CORE_SCHEDULER_SERVICES_H_
#define _CORE_SCHEDULER_SERVICES_H_
#include <core/task/services.h>

bool scheduler_initialise(void);

/**
 * Return the current running task.
 */
task_t* scheduler_current_task(void);

/**
 * Set a task as ready to execute.
 */
bool scheduler_task_ready(task_t* task);

/**
 * Set a task to not ready status. If the task is scheduled,
 * remove the task from all processors.
 */
bool scheduler_task_not_ready(task_t* task);

/**
 * Look for a next task available to run.
 * This method doesn't returns.
 */
__attribute((noreturn)) void scheduler_switch_task();

#endif