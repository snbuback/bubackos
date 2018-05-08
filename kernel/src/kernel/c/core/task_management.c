#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <core/logging.h>
#include <core/task_management.h>
#include <core/configuration.h>
#include <core/memory_management.h>
#include <hal/native_task.h>

static volatile task_id_t last_id = 0;

void task_management_initialize(void) {
  log_debug("task_management_initialization started");
}

task_t* task_create()
{
  task_t* task = kmem_alloc(sizeof(task_t));
  memset(task, 0, sizeof(task_t));
  
  task->task_id = ++last_id;
  task->status = TASK_STATUS_CREATED;
  task->stack_size = TASK_DEFAULT_STACK_SIZE;
  task->stack_address = (uintptr_t) kmem_alloc(TASK_DEFAULT_STACK_SIZE);
  log_trace("Created task %d at %p", task->task_id, task);
  return task;
}

bool task_start(task_t *task, uintptr_t code)
{
  if (task->status != TASK_STATUS_CREATED) {
    log_warn("Started task %d with invalid status", task->task_id);
    return false;
  }

  hal_create_native_task(&task->native_task, code, task->stack_address + task->stack_size - sizeof(long));
  task->status = TASK_STATUS_READY;
  return true;
}
