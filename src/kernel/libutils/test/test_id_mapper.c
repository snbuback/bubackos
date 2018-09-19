// source: src/libutils/id_mapper.c
// source: src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <libutils/id_mapper.h>

void test_id_mapper_create_null_reference_allocates_new_address()
{
    id_mapper_t* idm = id_mapper_create(NULL);
    TEST_ASSERT_NOT_NULL(idm);
}

void test_id_mapper_create_use_allocated_address()
{
    id_mapper_t idm;
    id_mapper_t* idm_r = id_mapper_create(&idm);
    TEST_ASSERT_EQUAL_PTR(&idm, idm_r);
}

void test_next_id()
{
    id_mapper_t* idm = id_mapper_create(NULL);
    TEST_ASSERT_EQUAL(1, id_mapper_next_id(idm));
    TEST_ASSERT_EQUAL(2, id_mapper_next_id(idm));
    TEST_ASSERT_EQUAL(3, id_mapper_next_id(idm));
    TEST_ASSERT_EQUAL(4, id_mapper_next_id(idm));
}

void test_get_with_invalid_reference()
{
    TEST_ASSERT_NULL(id_mapper_get(NULL, 1));
}

void test_add_with_invalid_reference()
{
    int val = 0x3343;
    TEST_ASSERT_EQUAL(0, id_mapper_add(NULL, &val));

    id_mapper_t* idm = id_mapper_create(NULL);
    TEST_ASSERT_EQUAL(0, id_mapper_add(idm, NULL));
}

void test_get()
{
    int val = 0x3784a;
    id_mapper_t idm;
    id_mapper_create(&idm);
    id_handler_t id = id_mapper_add(&idm, &val);
    TEST_ASSERT_EQUAL_PTR(&val, id_mapper_get(&idm, id));
    TEST_ASSERT_NULL(id_mapper_get(&idm, id+1));
}

void test_add_and_get()
{
    int val = 0x3784a;
    int val3 = 0x42323;
    id_mapper_t idm;
    id_mapper_create(&idm);
    id_handler_t id1 = id_mapper_add(&idm, &val);
    // let skip one value
    id_handler_t id2 = id_mapper_next_id(&idm);
    id_handler_t id3 = id_mapper_add(&idm, &val3);
    TEST_ASSERT_EQUAL(id1+1, id2); // ensure the skip works

    // check get
    TEST_ASSERT_EQUAL_PTR(&val, id_mapper_get(&idm, id1));
    TEST_ASSERT_EQUAL_PTR(&val3, id_mapper_get(&idm, id3));
    TEST_ASSERT_NULL(id_mapper_get(&idm, id2)); // skipped value
    TEST_ASSERT_NULL(id_mapper_get(&idm, id3+1)); // next value
}

void test_del_invalid_reference()
{
    TEST_ASSERT_FALSE(id_mapper_del(NULL, 1));

    id_mapper_t* idm = id_mapper_create(NULL);
    TEST_ASSERT_FALSE(id_mapper_del(idm, 0));
}

void test_del()
{
    int val1 = 0x3784a;
    int val2 = 0x42323;
    id_mapper_t idm;
    id_mapper_create(&idm);
    id_handler_t id1 = id_mapper_add(&idm, &val1);
    id_handler_t id2 = id_mapper_add(&idm, &val2);

    // del and ensure is NULL
    TEST_ASSERT_TRUE(id_mapper_del(&idm, id1));
    TEST_ASSERT_NULL(id_mapper_get(&idm, id1));

    TEST_ASSERT_TRUE(id_mapper_del(&idm, id2));
    TEST_ASSERT_NULL(id_mapper_get(&idm, id2));
}
