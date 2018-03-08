#ifndef __KERNEL_SERVICES_H
#define __KERNEL_SERVICES_H
#include <stdint.h>

#define MAX_SERVICES 		    512

typedef void (*Syscall)(char *);

typedef uint64_t servicehandler_t;

typedef struct {
  char* name;
  Syscall call;
} SyscallInfo_t;

void services_initialize(void);
servicehandler_t services_register(SyscallInfo_t info);

// TODO this function signature needs change: return and arguments should be more generic
void services_call(servicehandler_t service_handler, char* service_arg1);

#endif
