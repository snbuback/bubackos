#include <hal/configuration.h>
#include <logging.h>
#include <core/memory.h>
#include <core/task_management.h>
#include <hal/native_task.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <algorithms/linkedlist.h>
#include <core/memory_management.h>

static volatile task_id_t last_id;
static linkedlist_t* task_list;
static volatile task_id_t current_task_id;

bool task_management_initialize(void)
{
    task_list = linkedlist_create();
    if (!task_list) {
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
    log_warn("total of tasks: %d", linkedlist_size(task_list));
    linkedlist_remove_element(task_list, task);
    log_warn("total of tasks (after): %d", linkedlist_size(task_list));

    if (task->name) {
        kfree(task->name);
    }

    // TODO missing release memory resources

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
    task->userdata = 0;
    task->memory_handler = memory_handler;

    if (name) {
        size_t name_size = strlen(name) + 1;
        char* new_name = (char*) kalloc(name_size);
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

/**
 * Compact argument data (copying) in the given memory.
 * In case there is no enough memory, NULL is returned (but the memory is partially filled)
 * 
 * Structure:
 *      array(char*, num_arguments) -> pointer to each argument. size = sizeof(argument_t) * num_arguments)
 *      NULL
 *      argument[0] (ending with \0)
 *      argument[1] (ending with \0)
 *      ...
 *      task_userdata_t
 * 
 */
uintptr_t copy_arguments_to_task(task_t* task, uintptr_t stack, size_t stack_size, size_t num_arguments, const char* arguments[])
{
    // first element is the vector with all arguments position on the stack
    uintptr_t arguments_ptr = stack;
    argument_t* arguments_ptr_phys = (argument_t*) memory_management_get_physical_address(task->memory_handler, arguments_ptr);
    if (!arguments_ptr_phys) {
        log_error("Error finding the physical memory address of %p", arguments_ptr_phys);
        return 0;
    }

    // arguments data pointer
    uintptr_t arguments_data = stack + sizeof(argument_t) * (num_arguments + 1); // ends with NULL
    void* arguments_data_phys = (void*) memory_management_get_physical_address(task->memory_handler, arguments_data);
    if (!arguments_data_phys) {
        log_error("Error finding the physical memory address of %p", arguments_data);
        return 0;
    }

    for (size_t i=0; i<num_arguments; ++i) {
        // copy argument to the program data area
        size_t data_size = strlen(arguments[i]) + 1; // + null terminator

        // check if there is enough space for the data + num
        if (arguments_data + data_size > (uintptr_t) stack + stack_size - sizeof(task_userdata_t)) {
            return 0;
        }

        memcpy(arguments_data_phys, arguments[i], data_size);

        arguments_ptr_phys[i] = arguments_data;

        // increments
        arguments_data += data_size;
        arguments_data_phys += data_size;
    }

    // now stores the task_userdata_t
    task_userdata_t* userdata = (task_userdata_t*) arguments_data_phys;
    userdata->num_arguments = num_arguments;
    userdata->argument_list_ptr = arguments_ptr;
    return arguments_data; // task_userdata_t is stored virtually on arguments_data
}

bool task_set_arguments(task_id_t task_id, size_t num_arguments, const char* arguments[])
{
    task_t* task = get_task(task_id);
    if (task == NULL || task->status != TASK_STATUS_CREATED || task->userdata) {
        log_warn("Invalid task (or status) to set arguments: %d", task_id);
        return false;
    }

    // TODO fix permissions
    memory_region_t* region = memory_management_region_create(task->memory_handler, "?-arguments", 0, TASK_DEFAULT_STACK_SIZE, true, true, true);
    if (!region) {
        log_warn("Error allocating userdata for task %d", task_id);
        return false;
    }
    log_debug("Task data allocated at %p with size %d", region->start, region->size);

    task->userdata = copy_arguments_to_task(task, region->start, region->size, num_arguments, arguments);
    return true;
}

bool task_start(task_id_t task_id, uintptr_t code)
{
    task_t* task = get_task(task_id);
    if (task == NULL || task->status != TASK_STATUS_CREATED) {
        log_warn("Invalid task to start: %d", task_id);
        return false;
    }

    memory_region_t* region = memory_management_region_create(task->memory_handler, "?-stack", 0, TASK_DEFAULT_STACK_SIZE, true, true, true);
    if (!region) {
        log_warn("Error allocating stack address for task %d", task_id);
        return false;
    }

    // missing add the userdata parameter
    hal_create_native_task(&task->native_task, code, region->start + region->size - 8, task->kernel, task->userdata);
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

    log_trace("total of tasks (get_next_task): %d", linkedlist_size(task_list));
    WHILE_LINKEDLIST_ITER(task_list, task_t*, task) {
        log_debug("Checking task %s [%d] task priority=%d min priority=%d", task->name, task->task_id, task->priority, min_priority);
        if (task->status == TASK_STATUS_READY) {
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
        log_trace("Switching to task %d. Code at %p, stack at %p", current_task_id, task->native_task.codeptr, task->native_task.stackptr);
        // memory_management_dump(task->memory_handler);
        native_pagetable_switch(task->memory_handler->pt);
        hal_switch_task(&task->native_task);
    } else {
        // halt until a new event
        current_task_id = 0;
        log_trace("Sleeping...zzz");
        hal_sleep();
    }
}