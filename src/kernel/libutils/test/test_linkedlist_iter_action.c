// source: src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <stdio.h>
#include <string.h>
#include <algorithms/linkedlist.h>

static linkedlist_t* ll;
char* my_data[] = { "my data1", "my-data2", "mydata3" };
void* my_function_data = "my data";

void setUp()
{
    ll = linkedlist_create();
    linkedlist_append(ll, my_data[0]);
    linkedlist_append(ll, my_data[1]);
    linkedlist_append(ll, my_data[2]);
}

void tearDown()
{
    linkedlist_destroy(ll);
    ll = NULL;
}

// finding an existing element //
static int _test_finding(void* data, void* el)
{
    if (el == data) {
        return ITER_STOP_AND_RETURN;
    }
    return ITER_GO_NEXT;
}

void test_linkedlist_iter_finding_element()
{
    TEST_ASSERT_EQUAL_PTR(my_data[2], linkedlist_iter_with_action(ll, _test_finding, my_data[2]));
}

// finding an non-existing element //
static int _test_no_finding(void* data, void* el)
{
    return ITER_GO_NEXT;
}

void test_linkedlist_iter_non_existing_element()
{
    TEST_ASSERT_NULL(linkedlist_iter_with_action(ll, _test_no_finding, my_function_data));
}

// remove and return an element //
static int _test_remove_el(void* data, void* el)
{
    if (el == data) {
        return ITER_REMOVE_AND_RETURN;
    }
    return ITER_GO_NEXT;
}

void test_linkedlist_iter_remove_and_return()
{
    TEST_ASSERT_EQUAL_PTR(my_data[2], linkedlist_iter_with_action(ll, _test_remove_el, my_data[2]));
    TEST_ASSERT_EQUAL(-1, linkedlist_find(ll, my_data[2]));
}

// remove and continue //
static int _test_remove_and_continue(void* data, void* el)
{
    if (el == data) {
        return ITER_REMOVE_AND_NEXT;
    }
    return ITER_GO_NEXT;
}

void test_linkedlist_iter_remove_and_continue()
{
    TEST_ASSERT_NULL(linkedlist_iter_with_action(ll, _test_remove_and_continue, my_data[2]));
    TEST_ASSERT_EQUAL(-1, linkedlist_find(ll, my_data[2]));
}


