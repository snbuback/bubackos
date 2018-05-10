#include <core/configuration.h>
#include <core/logging.h>
#include <core/memory_management.h>
#include <core/task_management.h>
#include <hal/native_task.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static volatile task_id_t last_id;
static task_t* task_list[SYSTEM_LIMIT_OF_TASKS];
static volatile task_id_t current_task_id;

void task_management_initialize(void)
{
    memset(task_list, 0, sizeof(task_list));
    last_id = 0;
    current_task_id = 0;
    log_debug("task_management_initialization started");
}

static inline task_t* get_task(task_id_t task_id)
{
    if (task_id >= SYSTEM_LIMIT_OF_TASKS) {
        return NULL_TASK;
    }
    return task_list[task_id];
}

task_id_t get_current_task(void)
{
    return current_task_id;
}

task_id_t task_create()
{
    task_id_t task_id = ++last_id;
    if (task_id >= SYSTEM_LIMIT_OF_TASKS) {
        log_fatal("Limit of tasks reached: %d", task_id);
        return NULL_TASK;
    }

    task_t* task = kmem_alloc(sizeof(task_t));
    memset(task, 0, sizeof(task_t));

    task->task_id = task_id;
    task->priority = 1;
    task->status = TASK_STATUS_CREATED;
    task->stack_size = TASK_DEFAULT_STACK_SIZE;
    task->stack_address = (uintptr_t)kmem_alloc(TASK_DEFAULT_STACK_SIZE);
    task_list[task->task_id] = task;
    log_trace("Created task %d at %p", task->task_id, task);
    return task->task_id;
}

bool task_start(task_id_t task_id, uintptr_t code)
{
    task_t* task = get_task(task_id);
    if (task == NULL || task->status != TASK_STATUS_CREATED) {
        log_warn("Invalid task to start: %d", task_id);
        return false;
    }

    hal_create_native_task(&task->native_task, code, task->stack_address + task->stack_size - sizeof(long));
    task->status = TASK_STATUS_READY;
    return true;
}

bool task_destroy(task_id_t task_id)
{
    task_t* task = get_task(task_id);
    if (task == NULL) {
        log_warn("Invalid task to destroy: %d", task_id);
        return false;
    }
    // TODO clean up task data
    kmem_free(task);
    task_list[task_id] = NULL;
    return true;
}

void task_update_current_state(native_task_t *native_task)
{
    task_t* task = get_task(get_current_task());
    if (task == NULL) {
        // no status to update
        return;
    }

    // copy content from native_task
    task->native_task = *native_task;
}

/**
 * Get the next task to execute. The current implementation tries to avoid select the same task
 */
static task_t* get_next_task()
{
    task_priority_t min_priority = 0;
    task_t* task_to_switch = NULL_TASK;

    task_id_t task_id = 0;
    while (++task_id <= last_id) {
        task_t* task = get_task(task_id);
        if (task != NULL && task->status == TASK_STATUS_READY) {
            // check the priority
            if (task->priority < min_priority || task_to_switch == NULL_TASK) {
                min_priority = task->priority;
                task_to_switch = task;
            }
        }
    }
    return task_to_switch;
}

/**
 * This function should NEVER ever returns!
 */
void do_task_switch()
{
    task_t* task = get_next_task();
    if (task != NULL) {
        ++task->priority;
        current_task_id = task->task_id;
        hal_switch_task(&task->native_task);
    } else {
        // halt until a new event
        current_task_id = 0;
        hal_sleep();
    }
}