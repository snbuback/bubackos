#ifndef _CORE_SYSCALL_H_
#define _CORE_SYSCALL_H_
#include <hal/native_task.h>

long do_syscall(native_task_t *native_task, long syscall_number);

#endif