#include <stdbool.h>
#include <kernel/task_management.h>
#include <kernel/configuration.h>
#include <system.h>

task_t tasks[SYSTEM_LIMIT_OF_TASKS];
volatile task_handler_t last_task_handler = 0;


void task_management_initialize(void) {
  kprintf("task_management_initialization started");
}

task_handler_t __allocate_new_task(void) {
  return ++last_task_handler;
}

task_t* __get_task(task_handler_t task_handler) {
  task_t* task = &tasks[task_handler];
  return task;
}

uint32_t task_create(memory_handler_t memory_handler) {
  task_handler_t task_handler = __allocate_new_task();
  task_t* task = __get_task(task_handler);
  task->task_handler = task_handler;
  task->memory_handler = memory_handler;
}

bool task_start(task_handler_t task_handler, void* start_code_address) {
  task_t* task = __get_task(task_handler);
  task->next_address = start_code_address;
  task->ready = true;
}

void task_switch(task_handler_t task_handler) {
  task_t* task = __get_task(task_handler);

  _platform_task_switch(task->next_address);
  return;
}

void task_destroy(task_handler_t task_handler) {
  task_t* task = __get_task(task_handler);
  task->ready = false;

  // pending ensure the task is not running in another processors
}
