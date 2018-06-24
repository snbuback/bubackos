// source: kernel/c/core/task_management.c
#include <kernel_test.h>
#include <core/task_management.h>
#include <hal/native_task.h>

// pointer to 0x1 means hal_sleep called
#define TASK_SWITCH_SLEEP       ((native_task_t*) 0x1)
#define EFLAGS_TEST             0x123456
static native_task_t* task_switch_called = NULL;

// do_task_switch is a non-return function. To have the "same" behaviour I'm using longjmp to return to the test function
#define DO_TASK_SWITCH()         if (!setjmp(state)) { do_task_switch(); }
static jmp_buf state;

native_page_table_t* hal_page_table_create_mapping()
{
    return NULL;
}

void hal_switch_task(native_task_t *task)
{
    TEST_ASSERT_NULL(task_switch_called);
    task_switch_called = task;
    longjmp(state, 1);
}

void hal_sleep(void)
{
    TEST_ASSERT_NULL(task_switch_called);
    task_switch_called = TASK_SWITCH_SLEEP;
    longjmp(state, 2);
}

// Unit tests
void setUp(void)
{
    setvbuf(stdout, NULL, _IONBF, 0); // no printf buffer
    task_switch_called = NULL;
    task_management_initialize(); // clear the task list
}

void test_new_task_are_not_ready_to_execute(void)
{
    task_id_t id = task_create("test1", NULL);
    TEST_ASSERT_GREATER_OR_EQUAL(1, id);
    DO_TASK_SWITCH();    
    TEST_ASSERT_EQUAL(TASK_SWITCH_SLEEP, task_switch_called);
}
