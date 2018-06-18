// source: kernel/c/hal/x86_64/native_memory.c
#include <stdbool.h>
#include <kernel_test.h>
#include <hal/hal.h>
#include <string.h>

// tests
void test_create_entries_is_memory_aligned()
{
    // creates some allocation and ensure all of them are memory align
    for (int i=0; i<10; i++) {
        uintptr_t addr = (uintptr_t) create_entries();
        uintptr_t addr_align = ALIGN(addr, PAGE_TABLE_ALIGN);
        TEST_ASSERT_EQUAL_HEX(addr, addr_align);
        kmem_alloc(i); // insert some unliagned in memory allocation
    }
}

void test_index_for_level()
{
    // level 1
    TEST_ASSERT_EQUAL_INT(0, index_for_level(1, 0x0));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(1, 0x1000));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(1, 0x2000-1));
    TEST_ASSERT_EQUAL_INT(2, index_for_level(1, 0x2000));

    // level 2
    TEST_ASSERT_EQUAL_INT(0, index_for_level(2, 0x0));
    TEST_ASSERT_EQUAL_INT(0, index_for_level(2, 0x200000-1));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(2, 0x200000));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(2, 0x200000+1));

    // level 3
    TEST_ASSERT_EQUAL_INT(0, index_for_level(3, 0x0));
    TEST_ASSERT_EQUAL_INT(0, index_for_level(3, 0x40000000-1));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(3, 0x40000000));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(3, 0x40000000+1));

    // level 3
    TEST_ASSERT_EQUAL_INT(0, index_for_level(4, 0x0));
    TEST_ASSERT_EQUAL_INT(0, index_for_level(4, 0x8000000000-1));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(4, 0x8000000000));
    TEST_ASSERT_EQUAL_INT(1, index_for_level(4, 0x8000000000+1));

    // especific address
    uintptr_t ptr = 0xE6A2552A6000;
    TEST_ASSERT_EQUAL_INT(166, index_for_level(1, ptr));
    TEST_ASSERT_EQUAL_INT(169, index_for_level(2, ptr));
    TEST_ASSERT_EQUAL_INT(137, index_for_level(3, ptr));
    TEST_ASSERT_EQUAL_INT(461, index_for_level(4, ptr));
}

#define TEST_ASSERT_PAGE_TABLE(expected, entry)     TEST_ASSERT_EQUAL_UINT64(expected, (*(uint64_t*) entry))

void test_get_entry_value()
{
    page_entry_t entry;

    // TODO Tests not working
    memset(&entry, 0, sizeof(entry));
    fill_entry_value(&entry, 0x0, false, true, true);
    TEST_ASSERT_PAGE_TABLE(0x3, &entry);

    memset(&entry, 0, sizeof(entry));
    fill_entry_value(&entry, 0x0, false, true, false);
    TEST_ASSERT_PAGE_TABLE(0x1, &entry);

    memset(&entry, 0, sizeof(entry));
    fill_entry_value(&entry, 0x0, true, false, true);
    TEST_ASSERT_PAGE_TABLE(0x8000000000000007, &entry);

    memset(&entry, 0, sizeof(entry));
    fill_entry_value(&entry, 0x0, false, false, true);
    TEST_ASSERT_PAGE_TABLE(0x8000000000000003, &entry);

    uintptr_t ptr = 0x525224245000;
    memset(&entry, 0, sizeof(entry));
    fill_entry_value(&entry, ptr, true, true, false);
    TEST_ASSERT_PAGE_TABLE(0x525224245005, &entry);
}

void test_hal_page_table_create_mapping_returns_non_null()
{
    TEST_ASSERT_NOT_NULL(hal_page_table_create_mapping());
}

void test_hal_page_table_add_mapping()
{
    native_page_table_t* hal_mmap = hal_page_table_create_mapping();
    hal_page_table_add_mapping(hal_mmap, 0x120000, 0x320000, true, true, true);
    parse_intel_memory(hal_mmap->entries, NULL);
}