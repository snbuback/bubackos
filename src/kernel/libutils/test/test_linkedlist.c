// source: src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <stdio.h>
#include <string.h>
#include <algorithms/linkedlist.h>

void test_linkedlist_create()
{
    linkedlist_t* ll = linkedlist_create();
    TEST_ASSERT_EQUAL(0, ll->size);
    TEST_ASSERT_NULL(ll->first);
}

void test_linkedlist_destroy_empty_list()
{
    linkedlist_t* ll = linkedlist_create();
    linkedlist_destroy(ll);
}

void test_linkedlist_destroy_list_with_many_elements()
{
    char *data = "any pointer";
    linkedlist_t* ll = linkedlist_create();
    // do that with a larger list
    for (int i=0; i<1000; i++) {
        linkedlist_append(ll, data);
    }
    linkedlist_destroy(ll);
}

void test_linkedlist_append_empty_list()
{
    char* my_data = "my data";
    linkedlist_t* ll = linkedlist_create();
    linkedlist_append(ll, my_data);
    TEST_ASSERT_EQUAL(1, ll->size);
    TEST_ASSERT_NOT_NULL(ll->first);
    TEST_ASSERT_EQUAL(my_data, ll->first->val);
    TEST_ASSERT_NULL(ll->first->next);
    linkedlist_destroy(ll);
}

void test_linkedlist_append_3_elements()
{
    char* my_data[] = { "my data1", "my-data2", "mydata3"};
    linkedlist_t* ll = linkedlist_create();
    linkedlist_append(ll, my_data[0]);
    linkedlist_append(ll, my_data[1]);
    linkedlist_append(ll, my_data[2]);
    TEST_ASSERT_EQUAL(3, ll->size);

    // first element
    TEST_ASSERT_NOT_NULL(ll->first);
    TEST_ASSERT_EQUAL(my_data[0], ll->first->val);
    TEST_ASSERT_NOT_NULL(ll->first->next);

    // second element
    TEST_ASSERT_EQUAL(my_data[1], ll->first->next->val);
    TEST_ASSERT_NOT_NULL(ll->first->next->next);

    // third element
    TEST_ASSERT_EQUAL(my_data[2], ll->first->next->next->val);
    TEST_ASSERT_NULL(ll->first->next->next->next);
    linkedlist_destroy(ll);
}

void test_linkedlist_iterate_empty_list()
{
    linkedlist_t* ll = linkedlist_create();

    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    TEST_ASSERT_NULL(linkedlist_iter_next(&iter));
    linkedlist_destroy(ll);
}

void test_linkedlist_iterate_over_1_element()
{
    char* my_data = "my data";
    linkedlist_t* ll = linkedlist_create();
    linkedlist_append(ll, my_data);

    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    TEST_ASSERT_EQUAL(my_data, linkedlist_iter_next(&iter));
    TEST_ASSERT_NULL(linkedlist_iter_next(&iter));
    linkedlist_destroy(ll);
}

void test_linkedlist_iterate_over_3_elements()
{
    char* my_data[] = { "my data1", "my-data2", "mydata3"};
    linkedlist_t* ll = linkedlist_create();
    linkedlist_append(ll, my_data[0]);
    linkedlist_append(ll, my_data[1]);
    linkedlist_append(ll, my_data[2]);

    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    TEST_ASSERT_EQUAL(my_data[0], linkedlist_iter_next(&iter));
    TEST_ASSERT_EQUAL(my_data[1], linkedlist_iter_next(&iter));
    TEST_ASSERT_EQUAL(my_data[2], linkedlist_iter_next(&iter));
    TEST_ASSERT_NULL(linkedlist_iter_next(&iter));
    linkedlist_destroy(ll);
}

void test_linkedlist_pop_empty_list()
{
    linkedlist_t* ll = linkedlist_create();
    TEST_ASSERT_NULL(linkedlist_pop(ll));
    linkedlist_destroy(ll);
}

void test_linkedlist_pop_3_elements_list()
{
    char* my_data[] = { "my data1", "my-data2", "mydata3"};
    linkedlist_t* ll = linkedlist_create();
    linkedlist_append(ll, my_data[0]);
    linkedlist_append(ll, my_data[1]);
    linkedlist_append(ll, my_data[2]);

    TEST_ASSERT_EQUAL(my_data[2], linkedlist_pop(ll));
    TEST_ASSERT_EQUAL(2, ll->size);
    TEST_ASSERT_EQUAL(my_data[1], linkedlist_pop(ll));
    TEST_ASSERT_EQUAL(1, ll->size);
    TEST_ASSERT_EQUAL(my_data[0], linkedlist_pop(ll));
    TEST_ASSERT_EQUAL(0, ll->size);
    TEST_ASSERT_NULL(linkedlist_pop(ll));
    TEST_ASSERT_EQUAL(0, ll->size); // ensure size is still 0
    linkedlist_destroy(ll);
}

void test_linkedlist_get()
{
    char* my_data[] = { "my data1", "my-data2", "mydata3"};
    linkedlist_t* ll = linkedlist_create();
    linkedlist_append(ll, my_data[0]);
    linkedlist_append(ll, my_data[1]);
    linkedlist_append(ll, my_data[2]);

    TEST_ASSERT_EQUAL(my_data[2], linkedlist_get(ll, 2));
    TEST_ASSERT_EQUAL(my_data[1], linkedlist_get(ll, 1));
    TEST_ASSERT_EQUAL(my_data[0], linkedlist_get(ll, 0));
    TEST_ASSERT_NULL(linkedlist_get(ll, 3));
    linkedlist_destroy(ll);
}
