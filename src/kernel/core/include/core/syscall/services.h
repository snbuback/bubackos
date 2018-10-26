#ifndef _CORE_SYSCALL_SERVICES_H_
#define _CORE_SYSCALL_SERVICES_H_
#include <stdbool.h>

typedef long (*syscall_handler_func)(long arg1, long arg2, long arg3, long arg4, long arg5);

bool syscall_initialize();

bool register_syscall(unsigned long syscall_number, syscall_handler_func handler);

// handle_syscall is defined in hal, hw_events

bool syscall_registering();

#endif