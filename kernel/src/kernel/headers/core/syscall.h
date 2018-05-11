#ifndef _CORE_SYSCALL_H_
#define _CORE_SYSCALL_H_

typedef long (*syscall_handler_t)(long syscall_number);

long syscall(long syscall_number);

#endif