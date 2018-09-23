#ifndef _CORE_TASK_SERVICES_H_
#define _CORE_TASK_SERVICES_H_
#include <stdint.h>
#include <libutils/id_mapper.h>
#include <core/vmem/services.h>
#include <hal/native_task.h>

typedef id_handler_t task_id_t;
typedef unsigned int task_priority_t;

typedef enum { TASK_STATUS_CREATED, TASK_STATUS_READY, TASK_STATUS_NOT_READY } task_status_t;

typedef struct {
    task_id_t task_id;
    const char* name;
    bool kernel;
    task_priority_t priority;
    uintptr_t userdata;
    task_status_t status;
    native_task_t native_task;
    vmem_t* memory_handler;
} task_t;

typedef struct {
    // TODO change to use binary data instead of strings.
    unsigned long num_arguments;
    uintptr_t argument_list_ptr; // char* arguments[]
} task_userdata_t;

/**
 * Initialise task management service.
 */
bool task_service_initialize(void);

/**
 * Returns the task_t structure associate to the task_id.
 */
task_t* task_get(task_id_t task_id);

/**
 * Returns the task name when available or "(invalid task)".
 * This function always returns a string, even when the task is invalid.
 */
static inline const char* task_display_name(task_t* task)
{
    if (!task) {
        return "(invalid task)";
    } else if (!task->name) {
        return "(unamed task)";
    }
    return task->name;
}

/**
 * Creates a new task.
 */
task_t* task_create(const char* name, vmem_t* memory_handler);

/**
 * Set the task to run in kernel mode.
 */
bool task_set_kernel_mode(task_t* task);

/**
 * Configure the task arguments.
 */
bool task_set_arguments(task_t* task, size_t num_arguments, const size_t* sizes, const char* arguments[]);

/**
 * Start (or restart) a task.
 * @param code: the address of the task to execute. This address is relative to the task virtual memory.
 */
bool task_run(task_t* task, uintptr_t code);

/**
 * Destroy a task.
 * Note: destroy the task doesn't release the memory handler.
 */
bool task_destroy(task_t* task);


#endif