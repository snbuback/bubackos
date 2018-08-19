#include <hal/configuration.h>
#include <logging.h>
#include <core/memory.h>
#include <core/task_management.h>
#include <hal/native_task.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <algorithms/linkedlist.h>

static volatile task_id_t last_id;
static linkedlist_t* task_list;
static volatile task_id_t current_task_id;

bool task_management_initialize(void)
{
    task_list = linkedlist_create();
    if (task_list) {
        log_fatal("Error allocating memory for list of tasks");
        return false;
    }
    last_id = 0;
    current_task_id = 0;
    log_debug("task_management_initialization started");
    return true;
}

static inline task_t* get_task(task_id_t task_id)
{
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(task_list, &iter);
    task_t* task;
    while ((task = linkedlist_iter_next(&iter))) {
        if (task->task_id == task_id) {
            return task;
        }
    }
    return NULL_TASK;
}

task_id_t get_current_task(void)
{
    return current_task_id;
}

/**
 * Release all resources allocate by a task, inclusive the task_metadata itself
 */
static void task_release_resources(task_t* task)
{
    if (!task) {
        // no resources allocated
        return;
    }

    task_id_t task_id = task->task_id;

    // remove task from the task_list
    linkedlist_remove_element(task_list, task);

    if (task->name) {
        free(task->name);
    }

    // TODO missing release stack address

    // if the task is running, remove from it from current task.
    if (current_task_id == task_id) {
        current_task_id = 0;
    }
}

task_id_t task_create(const char* name, memory_t* memory_handler)
{
    task_id_t task_id = ++last_id;

    task_t* task = NEW(task_t);
    if (!task) {
        log_fatal("Unable to allocate memory for new task");
        return NULL_TASK;
    }

    task->task_id = task_id;
    task->kernel = false;
    task->priority = 1;
    task->status = TASK_STATUS_CREATED;
    task->stack_address = 0;
    task->memory_handler = memory_handler;

    if (name) {
        size_t name_size = strlen(name) + 1;
        char* new_name = (char*) malloc(name_size);
        strncpy(new_name, name, name_size);
        task->name = new_name;
    }

    // append task to the list
    if (!linkedlist_append(task_list, task)) {
        log_fatal("Unable to allocate memory for insert a new task");
        task_release_resources(task);
        return NULL_TASK;
    }
    log_trace("Created task %d with name %s", task->task_id, task->name);
    return task->task_id;
}

bool task_set_kernel_mode(task_id_t task_id)
{
    task_t* task = get_task(task_id);
    if (task == NULL || task->status != TASK_STATUS_CREATED) {
        log_warn("Invalid task status: %d", task_id);
        return false;
    }
    task->kernel = true;
    return true;
}

bool task_start(task_id_t task_id, uintptr_t code, uintptr_t stack)
{
    task_t* task = get_task(task_id);
    if (task == NULL || task->status != TASK_STATUS_CREATED) {
        log_warn("Invalid task to start: %d", task_id);
        return false;
    }

    task->stack_address = stack;
    hal_create_native_task(&task->native_task, code, task->stack_address, task->kernel);
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
    log_debug("Destroying task %d", task_id);
    task_release_resources(task);
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
static volatile task_id_t last_context_switch = 0;
void do_task_switch()
{
    task_t* task = get_next_task();
    if (task != NULL) {
        ++task->priority;
        current_task_id = task->task_id;
        if (current_task_id != last_context_switch) {
            log_trace("Switching to task %d", current_task_id);
            last_context_switch = current_task_id;
        }
        native_pagetable_switch(task->memory_handler->pt);
        hal_switch_task(&task->native_task);
    } else {
        // halt until a new event
        current_task_id = 0;
        if (current_task_id != last_context_switch) {
            log_trace("Sleeping...zzz");
            last_context_switch = 0;
        }
        hal_sleep();
    }
}