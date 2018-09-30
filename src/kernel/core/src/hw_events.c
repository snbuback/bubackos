#include <logging.h>
#include <core/hal/hw_events.h>
#include <core/task/services.h>
#include <core/scheduler/services.h>

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
    log_fatal("PF: addr=%p r=%d w=%d perm=%d instr=%d task=%s",
        pf.addr,
        pf.reading,
        pf.writing,
        pf.invalid_permission,
        pf.instruction_fetch,
        task_display_name(task));

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