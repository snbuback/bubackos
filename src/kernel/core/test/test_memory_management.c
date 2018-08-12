// source: src/memory_management.c
// source: ../libutils/src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <core/memory_management.h>

// mocks
#define CALLED_HAL_PAGE_TABLE     0x1034343
#define CALLED_PAGE_ALLOCATE      0x2649739
native_page_table_t* native_pagetable_create()
{
    return (void*) CALLED_HAL_PAGE_TABLE;
}

uintptr_t page_allocator_allocate()
{
    return CALLED_PAGE_ALLOCATE;
}

void native_pagetable_set(native_page_table_t* pt, page_map_entry_t entry)
{
    // do nothing
}

// tests
void test_create()
{
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_GREATER_OR_EQUAL(1, m->id);
    TEST_ASSERT_EQUAL(CALLED_HAL_PAGE_TABLE, m->pt);
    TEST_ASSERT_NOT_NULL(m->regions);
    TEST_ASSERT_NOT_NULL(m->map);
    // missing check if the memory create is in the global list
}

void test_create_region()
{
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, 0x404000, 10, true, true, true);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL(m, r->memory);
    TEST_ASSERT_EQUAL_HEX(0x404000, r->start);
    TEST_ASSERT_EQUAL_HEX(10, r->size);
    TEST_ASSERT_EQUAL(true, r->user);
    TEST_ASSERT_EQUAL(true, r->writable);
    TEST_ASSERT_EQUAL(true, r->executable);
    TEST_ASSERT_NOT_NULL(r->pages);
}

void test_get_physical_address()
{
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    const uintptr_t test_addr = 0x40000;
    memory_region_t* r = memory_management_region_create(m, test_addr, 10, true, true, true);
    TEST_ASSERT_EQUAL_HEX64(CALLED_PAGE_ALLOCATE, memory_management_get_physical_address(m, test_addr));
    TEST_ASSERT_EQUAL_HEX64(CALLED_PAGE_ALLOCATE+5, memory_management_get_physical_address(m, test_addr+5));
}