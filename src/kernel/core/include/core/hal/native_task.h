#ifndef _CORE_HAL_NATIVE_TASK_H_
#define _CORE_HAL_NATIVE_TASK_H_
#include <stdint.h>
#include <core/task/services.h>

bool native_task_create(task_t* task, uintptr_t code, uintptr_t stack, int permission_mode, uintptr_t userdata);

/**
 * Switch to a specific task.
 * Switches the page table mapping before switch the task.
 * If task is NULL, the processor should sleep until next hardware event.
 */
__attribute__ ((noreturn)) void native_task_switch(task_t* task);

/**
 * Sleeps the processor until next hardware event.
 */
__attribute__ ((noreturn)) void native_task_sleep(void);

#endif