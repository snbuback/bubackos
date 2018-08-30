#ifndef _CORE_TASK_MANAGEMENT_H
#define _CORE_TASK_MANAGEMENT_H
#include <stdint.h>
#include <stdbool.h>
#include <hal/native_task.h>
#include <core/memory_management.h>
#include <hal/configuration.h>

#define TASK_DEFAULT_STACK_SIZE  SYSTEM_PAGE_SIZE*10

typedef unsigned int task_id_t;
typedef unsigned int task_priority_t;

#define NULL_TASK         ((task_id_t) 0)

typedef enum { TASK_STATUS_CREATED, TASK_STATUS_READY } task_status_t;

typedef struct {
    task_id_t task_id;
    char* name;
    bool kernel;
    task_priority_t priority;
    uintptr_t userdata;
    task_status_t status;
    native_task_t native_task;
    memory_t* memory_handler;
} task_t;

typedef struct {
    argument_t num_arguments;
    argument_t argument_list_ptr; // char* arguments[]
} task_userdata_t;

bool task_management_initialize(void);

task_id_t get_current_task(void);

task_id_t task_create(const char* name, memory_t* memory_handler);

bool task_set_kernel_mode(task_id_t task_id);

bool task_set_arguments(task_id_t task_id, size_t num_arguments, const char* arguments[]);

bool task_start(task_id_t task_id, uintptr_t code);

void task_update_current_state(native_task_t *native_task);

// task_t* task_allocate(size_t code_size, size_t data_size, size_t stack_size);

// bool task_set_status(task_t* task, task_status_t status);

bool task_destroy(task_id_t task_id);

void do_task_switch() __attribute__ ((noreturn));

#endif