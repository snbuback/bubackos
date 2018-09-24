#include <stdbool.h>
#include <string.h>
#include <logging.h>
#include <libutils/id_mapper.h>
#include <algorithms/linkedlist.h>
#include <core/hal/platform.h>
#include <core/hal/native_task.h>
#include <core/alloc.h>
#include <core/vmem/services.h>
#include <core/scheduler/services.h>
#include <core/task/services.h>

static id_mapper_t task_id_mapper;

bool task_service_initialize(void)
{
    if (!id_mapper_create(&task_id_mapper)) {
        log_fatal("Error initialising task service");
        return false;
    }
    return true;
}

task_t* task_get(task_id_t task_id)
{
    return id_mapper_get(&task_id_mapper, task_id);
}

/**
 * Release all resources allocate by a task, inclusive the own task meta-data.
 */
static void task_release_resources(task_t* task)
{
    if (!task) {
        // no resources allocated
        return;
    }

    // ensure the task is not running on any other processor
    scheduler_task_not_ready(task);

    // remove task from the task_list
    id_mapper_del(&task_id_mapper, task->task_id);
}

task_t* task_create(const char* name, vmem_t* vmem)
{
    task_t* task = NEW(task_t);
    if (!task) {
        log_error("Unable to allocate memory for new task");
        return NULL;
    }

    *task = (task_t){
        .kernel = false,
        .priority = 1,
        .status = TASK_STATUS_CREATED,
        .userdata = 0,
        .memory_handler = vmem,
        .name = name
    };

    // append task to the list
    task->task_id = id_mapper_add(&task_id_mapper, task);
    if (!task->task_id) {
        goto error;
    }
    log_trace("Created task %d with name %s", task->task_id, task->name);
    return task;

error:
    log_error("Unable to allocate memory for insert a new task");
    FREE(task);
    return NULL;
}

bool task_set_kernel_mode(task_t* task)
{
    if (task == NULL || task->status != TASK_STATUS_CREATED) {
        log_warn("Invalid task status or reference: %d", task ? (int) task->status : -1);
        return false;
    }
    task->kernel = true;
    return true;
}

/**
 * Compact argument data (copying) in the given memory.
 * In case there is no enough memory, NULL is returned (but the memory is partially filled)
 * 
 * Structure:
 *      array(char*, num_arguments) -> pointer to each argument. size = sizeof(long) * num_arguments)
 *      NULL
 *      argument[0] (ending with \0)
 *      argument[1] (ending with \0)
 *      ...
 *      task_userdata_t
 * 
 */
uintptr_t copy_arguments_to_task(task_t* task, uintptr_t stack, size_t stack_size, size_t num_arguments, const size_t* sizes, const char* arguments[])
{
    // TODO Change to a simple serialization without pointers
    // first element is the vector with all arguments position on the stack
    uintptr_t arguments_ptr = stack;
    long* arguments_ptr_phys = (long*) vmem_get_physical_address(task->memory_handler, arguments_ptr);
    if (!arguments_ptr_phys) {
        log_error("Error finding the physical memory address of %p", arguments_ptr_phys);
        return 0;
    }

    // arguments data pointer
    uintptr_t arguments_data = stack + sizeof(long) * (num_arguments + 1); // ends with NULL
    void* arguments_data_phys = (void*) vmem_get_physical_address(task->memory_handler, arguments_data);
    if (!arguments_data_phys) {
        log_error("Error finding the physical memory address of %p", arguments_data);
        return 0;
    }

    for (size_t i=0; i<num_arguments; ++i) {
        // copy argument to the program data area
        size_t data_size = sizes[i];

        // check if there is enough space for the data + num
        if (arguments_data + data_size > (uintptr_t) stack + stack_size - sizeof(task_userdata_t)) {
            return 0;
        }

        memcpy(arguments_data_phys, arguments[i], data_size);

        arguments_ptr_phys[i] = arguments_data;

        // increments
        arguments_data += data_size;
        arguments_data_phys += data_size;
    }

    // now stores the task_userdata_t
    task_userdata_t* userdata = (task_userdata_t*) arguments_data_phys;
    userdata->num_arguments = num_arguments;
    userdata->argument_list_ptr = arguments_ptr;
    return arguments_data; // task_userdata_t is stored virtually on arguments_data
}

bool task_set_arguments(task_t* task, size_t num_arguments, const size_t* sizes, const char* arguments[])
{
    if (task == NULL || task->status != TASK_STATUS_CREATED || task->userdata) {
        log_warn("Invalid call to set arguments");
        return false;
    }

    // TODO fix permissions
    vmem_region_t* vmem_region = vmem_region_create(task->memory_handler, "task-argument", 0, TASK_DEFAULT_STACK_SIZE, true, true, true);
    if (!vmem_region) {
        log_warn("Error allocating userdata for task %d", task->task_id);
        return false;
    }
    log_debug("Task data allocated at %p with size %d", vmem_region->start, vmem_region->size);

    // attach the argument region to the kernel
    if (!vmem_attach(vmem_get_kernel(), vmem_region)) {
        return false;
    }
    task->userdata = copy_arguments_to_task(task, vmem_region->start, vmem_region->size, num_arguments, sizes, arguments);
    // TODO Detach argument region
    return true;
}

bool task_run(task_t* task, uintptr_t code)
{
    if (task == NULL || task->status != TASK_STATUS_CREATED) {
        log_warn("Invalid task status : %d", task ? task->status : 0);
        return false;
    }

    vmem_region_t* vmem_region = vmem_region_create(task->memory_handler, "?-stack", 0, TASK_DEFAULT_STACK_SIZE, true, true, true);
    if (!vmem_region) {
        log_warn("Error allocating stack address for task %d", task->task_id);
        return false;
    }

    // missing add the userdata parameter
    if (!native_task_create(task, code, vmem_region->start + vmem_region->size - 8, task->kernel, task->userdata)) {
        return false;
    }
    scheduler_task_ready(task);
    return true;
}

bool task_destroy(task_t* task)
{
    if (task == NULL) {
        log_warn("Invalid task to destroy");
        return false;
    }
    log_debug("Destroying task %d", task->task_id);
    task_release_resources(task);
    return true;
}
