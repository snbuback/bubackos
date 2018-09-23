// source: src/task/services.c
// source: src/scheduler/services.c
// source: src/vmem/services.c
// source: src/vmem/region_services.c
// source: src/pmem/services.c
// source: ../libutils/src/algorithms/linkedlist.c
// source: ../libutils/src/libutils/id_mapper.c
#include <string.h>
#include <kernel_test.h>
#include <core/task/services.h>
#include <hal/native_task.h>
#include <algorithms/linkedlist.h>
#include <core/hal/platform.h>

// pointer to 0x1 means native_task_sleep called
#define TASK_SWITCH_SLEEP       ((native_task_t*) 0x1)
#define EFLAGS_TEST             0x123456
static native_task_t* task_switch_called = NULL;

// schedule_task_switch is a non-return function. To have the "same" behaviour I'm using longjmp to return to the test function
#define DO_TASK_SWITCH()         if (!setjmp(state)) { scheduler_switch_task(); }
static jmp_buf state;

// test test_copy_arguments_to_task
#define STACK_SIZE  4096
char* STACK[STACK_SIZE];
#define STACK_BASE_ADDR   0x100000

void setUp() {
    vmem_initialize();
    vmem_region_initialize();
}

uintptr_t copy_arguments_to_task(task_t* task, uintptr_t stack, size_t stack_size, size_t num_arguments, const size_t* sizes, const char* arguments[]);

// TODO With changes planned this test probably needs to be rewritten
void xxtest_copy_arguments_to_task()
{
    const char* TEST_ARGUMENTS[] = {"first", "second", "third", "", "fifth", "with\nnew\nline"};
    const int TEST_NUM_ARGUMENTS = 6;
    size_t TEST_SIZES[TEST_NUM_ARGUMENTS];

    // fill TEST_SIZES
    for (int i=0; i<TEST_NUM_ARGUMENTS; i++) {
        TEST_SIZES[i] = strlen(TEST_ARGUMENTS[i]) + 1;
    }

    memset(STACK, '\0', STACK_SIZE);

    vmem_t memory_handler;
    task_t task = { .memory_handler = &memory_handler };

    // simulating stack virtual addr
    uintptr_t stack_virtual_addr = STACK_BASE_ADDR;
    uintptr_t userdata_vaddr = copy_arguments_to_task(&task, stack_virtual_addr, STACK_SIZE, TEST_NUM_ARGUMENTS, TEST_SIZES, TEST_ARGUMENTS);
    task_userdata_t* userdata = (task_userdata_t*) vmem_get_physical_address(task.memory_handler, userdata_vaddr);

    TEST_ASSERT_TRUE((uintptr_t) userdata > (uintptr_t) STACK && ((uintptr_t) userdata + sizeof(task_userdata_t)) < ((uintptr_t) STACK + STACK_SIZE))

    // validates the content
    TEST_ASSERT_EQUAL_HEX64(TEST_NUM_ARGUMENTS, userdata->num_arguments);
    uintptr_t* userdata_argument_list_ptr = (uintptr_t*) vmem_get_physical_address(task.memory_handler, userdata->argument_list_ptr);
    for (int i=0; i<TEST_NUM_ARGUMENTS; i++) {
        long val = (long) userdata_argument_list_ptr[i];
        char* param = (char*) vmem_get_physical_address(task.memory_handler, userdata_argument_list_ptr[i]);
        TEST_ASSERT_EQUAL_STRING(TEST_ARGUMENTS[i], param);
    }
}

