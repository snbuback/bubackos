// source: src/task_management.c
// source: ../libutils/src/algorithms/linkedlist.c
#include <string.h>
#include <kernel_test.h>
#include <core/task_management.h>
#include <hal/native_task.h>
#include <algorithms/linkedlist.h>
#include <hal/configuration.h>

// pointer to 0x1 means hal_sleep called
#define TASK_SWITCH_SLEEP       ((native_task_t*) 0x1)
#define EFLAGS_TEST             0x123456
static native_task_t* task_switch_called = NULL;

// do_task_switch is a non-return function. To have the "same" behaviour I'm using longjmp to return to the test function
#define DO_TASK_SWITCH()         if (!setjmp(state)) { do_task_switch(); }
static jmp_buf state;

// test test_copy_arguments_to_task
#define STACK_SIZE  4096
char* STACK[STACK_SIZE];
#define STACK_BASE_ADDR   0x100000

void hal_create_native_task(native_task_t *task, uintptr_t code, uintptr_t stack, int permission_mode, uintptr_t userdata)
{

}

void native_pagetable_switch(native_page_table_t* pt)
{
    
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

memory_region_t* memory_management_region_create(memory_t* memory, const char* region_name, uintptr_t start, size_t size, bool user, bool writable, bool executable)
{
    return NULL;
}

uintptr_t memory_management_get_physical_address(memory_t* mhandler, uintptr_t vaddr)
{
    return vaddr + (uintptr_t) &STACK[0] - STACK_BASE_ADDR;
}

void memory_management_dump(memory_t* memory)
{
    
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

uintptr_t copy_arguments_to_task(task_t* task, uintptr_t stack, size_t stack_size, size_t num_arguments, const char* arguments[]);

static char printch(char c)
{
    if (c>=32 && c<128) {
        return c;
    }
    return '#';
}

void test_copy_arguments_to_task()
{
    const char* TEST_ARGUMENTS[] = {"first", "second", "third", "", "fifth", "with\nnew\nline"};
    const int TEST_NUM_ARGUMENTS = 6;

    memset(STACK, '\0', STACK_SIZE);

    memory_t memory_handler;
    task_t task = { .memory_handler = &memory_handler };

    // simulating stack virtual addr
    uintptr_t stack_virtual_addr = STACK_BASE_ADDR;
    uintptr_t userdata_vaddr = copy_arguments_to_task(&task, stack_virtual_addr, STACK_SIZE, TEST_NUM_ARGUMENTS, TEST_ARGUMENTS);
    task_userdata_t* userdata = (task_userdata_t*) memory_management_get_physical_address(task.memory_handler, userdata_vaddr);

    TEST_ASSERT_TRUE((uintptr_t) userdata > (uintptr_t) STACK && ((uintptr_t) userdata + sizeof(task_userdata_t)) < ((uintptr_t) STACK + STACK_SIZE))

    // validates the content
    TEST_ASSERT_EQUAL_HEX64(TEST_NUM_ARGUMENTS, userdata->num_arguments);
    uintptr_t* userdata_argument_list_ptr = (uintptr_t*) memory_management_get_physical_address(task.memory_handler, userdata->argument_list_ptr);
    for (int i=0; i<TEST_NUM_ARGUMENTS; i++) {
        argument_t val = (argument_t) userdata_argument_list_ptr[i];
        char* param = (char*) memory_management_get_physical_address(task.memory_handler, userdata_argument_list_ptr[i]);
        TEST_ASSERT_EQUAL_STRING(TEST_ARGUMENTS[i], param);
    }
}

