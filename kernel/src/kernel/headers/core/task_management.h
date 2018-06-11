#ifndef _CORE_TASK_MANAGEMENT_H
#define _CORE_TASK_MANAGEMENT_H
#include <stdint.h>
#include <stdbool.h>
#include <hal/native_task.h>
#include <hal/hal.h>

#define TASK_DEFAULT_STACK_SIZE  1024

typedef unsigned int task_id_t;
typedef unsigned int task_priority_t;

#define NULL_TASK         ((task_id_t) 0)

typedef enum { TASK_STATUS_CREATED, TASK_STATUS_READY } task_status_t;

typedef struct {
  task_id_t task_id;
  char* name;
  task_priority_t priority;
  uintptr_t stack_address; // change to use memory management
  size_t    stack_size;
  task_status_t status;
  native_task_t native_task;
  native_page_table_t* native_page_table;
} task_t;

void task_management_initialize(void);

task_id_t get_current_task(void);

task_id_t task_create(char *name, native_page_table_t* native_page_table);

bool task_start(task_id_t task_id, uintptr_t code);

void task_update_current_state(native_task_t *native_task);

// task_t* task_allocate(size_t code_size, size_t data_size, size_t stack_size);

// bool task_set_status(task_t* task, task_status_t status);

bool task_destroy(task_id_t task_id);

void do_task_switch() __attribute__ ((noreturn));

#endif