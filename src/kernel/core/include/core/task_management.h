#ifndef _CORE_TASK_MANAGEMENT_H
#define _CORE_TASK_MANAGEMENT_H
#include <stdint.h>
#include <stdbool.h>
#include <hal/native_task.h>
#include <core/memory_management.h>
#include <hal/configuration.h>

#include <core/task/services.h>


native_task_t* task_update_current_state(native_task_t *native_task_on_stack);

#endif