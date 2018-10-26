#include <core/syscall/services.h>
#include <core/syscall/syscalls.h>
#include <logging.h>

// shortcut DEFINE to reduce the code declaration
#define SYSCALL_REGISTER(sysnumber, handler)      \
    { \
        extern long (*handler)(); \
        success = success && register_syscall(sysnumber, (syscall_handler_func) &handler); \
        log_trace("Registered syscall %d to %s (%p)", sysnumber, #handler, &handler); \
    }

bool syscall_registering()
{
    bool success = true;

    SYSCALL_REGISTER(SYSCALL_EXIT, syscall_handler_task_exit);
    SYSCALL_REGISTER(SYSCALL_LOGGING, syscall_handler_logging);

    return success;
}