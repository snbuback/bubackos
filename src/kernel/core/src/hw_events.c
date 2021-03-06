#include <logging.h>
#include <core/hal/hw_events.h>
#include <core/task/services.h>
#include <core/scheduler/services.h>
#include <core/vmem/services.h>
#include <core/hal/native_vmem.h>

void handle_protection_fault()
{
    task_t* task = scheduler_current_task();
    if (task) {
        log_fatal("General protection caused by task %s. Killing task.", task_display_name(task));
        task_destroy(task);
    } else {
        log_fatal("General protection caused by kernel :,(");
    }
    scheduler_switch_task();
}

void handle_page_fault(pagefault_status_t pf)
{
    task_t* task = scheduler_current_task();
    log_fatal("PF: addr=%p valid=%d !r=%d !w=%d !x=%d task=%s codeptr=%p stackptr=%p",
        pf.addr,
        pf.is_reference_valid,
        pf.no_reading_access,
        pf.no_writing_access,
        pf.no_execution_access,
        task_display_name(task),
        pf.codeptr,
        pf.stackptr
        );
    native_vmem_dump(NULL);

    if (task) {
        log_fatal("Page fault caused by task %s. Killing task.", task_display_name(task));
        task_destroy(task);
    } else {
        log_fatal("Page fault caused by kernel :,(");
    }
    scheduler_switch_task();
}

void handle_invalid_operation()
{
    log_error("Invalid operation");
    task_t* task = scheduler_current_task();
    if (task) {
        task_destroy(task);
    }
    scheduler_switch_task();
}

void handle_generic_hw_events()
{
    log_warn("Unknown hardware event");
}