#ifndef _CORE_TASK_MANAGEMENT_H
#define _CORE_TASK_MANAGEMENT_H
#include <stdint.h>
#include <stdbool.h>
#include <core/memory_allocator.h>
#include <hal/native_task.h>

#define TASK_DEFAULT_STACK_SIZE  1024

typedef unsigned int task_id_t;
typedef enum { TASK_STATUS_CREATED, TASK_STATUS_READY } task_status_t;

typedef struct {
  task_id_t task_id;
  uintptr_t stack_address; // change to use memory management
  size_t    stack_size;
  task_status_t status;
  native_task_t native_task;
} task_t;

void task_management_initialize(void);

task_t* task_create();

bool task_start(task_t *task, uintptr_t code);

// task_t* task_allocate(size_t code_size, size_t data_size, size_t stack_size);

// bool task_set_status(task_t* task, task_status_t status);

void task_destroy(task_t* task);

#endif