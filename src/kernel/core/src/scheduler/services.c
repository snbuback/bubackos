#include <logging.h>
#include <algorithms/linkedlist.h>
#include <core/vmem/services.h>
#include <core/hal/native_task.h>
#include <core/hal/native_vmem.h>
#include <core/scheduler/services.h>

static volatile task_t* current_task;

// contains all tasks ready to execute, unless task already in execution.
linkedlist_t* scheduled_tasks;

bool scheduler_initialise(void)
{
    scheduled_tasks = linkedlist_create();
    return scheduled_tasks != NULL;
}

inline task_t* scheduler_current_task(void)
{
    return (task_t*) current_task;
}

void scheduler_clear_current_task(void) {
    if (current_task) {
        if (current_task->status == TASK_STATUS_READY) {
            linkedlist_append(scheduled_tasks, (task_t*) current_task);
        }
        current_task = NULL;
    }
}

bool scheduler_task_not_ready(task_t* task)
{
    if (!task) {
        return false;
    }
    task->status = TASK_STATUS_NOT_READY;
    linkedlist_remove_element(scheduled_tasks, task);

    // remove from all processors
    if (scheduler_current_task() == task) {
        scheduler_clear_current_task();
    }
    return true;
}

bool scheduler_task_ready(task_t* task)
{
    if (!task) {
        return false;
    }
    task->status = TASK_STATUS_READY;
    linkedlist_append(scheduled_tasks, task);
    return true;
}

/**
 * Get the next task to execute, also remove it from list.
 */
static inline task_t* get_next_task()
{
    return linkedlist_remove(scheduled_tasks, 0);
}

__attribute((noreturn)) void scheduler_switch_task()
{
    task_t* next_task = get_next_task();
    if (next_task) {
        if (next_task != scheduler_current_task()) {
            // move last task to the list of ready to execute
            scheduler_clear_current_task();

            // switch page tables since it is a new different task
            // vmem_dump(task->memory_handler);
            native_vmem_switch(next_task->memory_handler);
        }
        current_task = next_task;
        native_task_switch(next_task);
    } else {
        // halt until a new event
        scheduler_clear_current_task();
        // log_trace("Sleeping...zzz");
        native_task_sleep();
    }
}

