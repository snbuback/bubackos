// source: src/memory_management.c
// source: ../libutils/src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <core/memory_management.h>

// mocks
#define CALLED_HAL_PAGE_TABLE     0x1034343
native_page_table_t* native_pagetable_create()
{
    return (void*) CALLED_HAL_PAGE_TABLE;
}

static uintptr_t last_page_allocated = SYSTEM_PAGE_SIZE * 1000;
uintptr_t page_allocator_allocate()
{
    return (last_page_allocated += SYSTEM_PAGE_SIZE);
}

bool page_allocator_mark_as_system(uintptr_t addr, size_t total_in_bytes)
{
    // nothing
    return true;
}

void native_pagetable_set(native_page_table_t* pt, page_map_entry_t entry)
{
    // do nothing
}

void native_page_table_flush()
{
    // nothing
}

void native_pagetable_switch(native_page_table_t* pt)
{
    
}

// tests
void test_create()
{
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_GREATER_OR_EQUAL(1, m->id);
    TEST_ASSERT_EQUAL(CALLED_HAL_PAGE_TABLE, m->pt);
    TEST_ASSERT_NOT_NULL(m->regions);
    TEST_ASSERT_EQUAL(0, linkedlist_size(m->regions));
    TEST_ASSERT_NOT_NULL(m->map);
    // missing check if the memory create is in the global list
}

void test_create_region()
{
    const char* name = "test-region";
    const intptr_t start_addr = 0x404000;
    const size_t num_pages = 20;
    const intptr_t size = 20*SYSTEM_PAGE_SIZE - 1;
    uintptr_t initial_paddr = last_page_allocated + SYSTEM_PAGE_SIZE;

    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, name, start_addr, size, true, true, true);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_NOT_EQUAL(-1, linkedlist_find(r->attached, m));
    TEST_ASSERT_EQUAL(name, r->region_name);
    TEST_ASSERT_EQUAL_HEX(start_addr, r->start);
    TEST_ASSERT_EQUAL_HEX(size, r->size);
    TEST_ASSERT_EQUAL_HEX(ALIGN_NEXT(size, SYSTEM_PAGE_SIZE), r->allocated_size);
    TEST_ASSERT_EQUAL(true, r->user);
    TEST_ASSERT_EQUAL(true, r->writable);
    TEST_ASSERT_EQUAL(true, r->executable);
    TEST_ASSERT_EQUAL(r, linkedlist_get(m->regions, 0));

    // verify if all pages are marked in the memory map
    TEST_ASSERT_EQUAL(num_pages, linkedlist_size(r->pages));
    for (size_t i=0; i<num_pages; ++i) {
        memory_map_t* map = linkedlist_get(m->map, i);
        TEST_ASSERT_NOT_NULL(map);
        TEST_ASSERT_EQUAL(initial_paddr + i*SYSTEM_PAGE_SIZE, map->physical_addr);
        TEST_ASSERT_EQUAL(r->start + i*SYSTEM_PAGE_SIZE, map->virtual_addr);
        TEST_ASSERT_EQUAL(r, map->region);
    }
}

void test_create_region_without_start_address_and_size_0()
{
    const char* name = "test-region";
    const intptr_t start_addr = 0;
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, name, start_addr, 0, true, true, true);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL(m, linkedlist_get(r->attached, 0));
    TEST_ASSERT_EQUAL(name, r->region_name);
    TEST_ASSERT_EQUAL_HEX(0x100000, r->start);
    TEST_ASSERT_EQUAL_HEX(0, r->size);
    TEST_ASSERT_EQUAL_HEX(0, r->allocated_size);
    TEST_ASSERT_EQUAL(true, r->user);
    TEST_ASSERT_EQUAL(true, r->writable);
    TEST_ASSERT_EQUAL(true, r->executable);
    TEST_ASSERT_EQUAL(0, linkedlist_size(r->pages));
}

void test_create_region_fail_with_unaligned_memory_address()
{
    const char* name = "test-region";
    const intptr_t start_addr = 0x404001;
    memory_t* m = memory_management_create();

    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, name, start_addr, 10, true, true, true);
    TEST_ASSERT_NULL(r);
}

void test_resize_region_allocates_new_pages()
{
    const char* name = "test-region";
    const size_t num_pages = 20;
    const intptr_t new_size = num_pages*SYSTEM_PAGE_SIZE;
    memory_t* m = memory_management_create();

    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, name, 0, 0, true, true, true);
    TEST_ASSERT_NOT_NULL(r);

    bool success = memory_management_region_resize(r, new_size);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL_HEX(new_size, r->size);
    TEST_ASSERT_EQUAL_HEX(new_size, r->allocated_size);
    TEST_ASSERT_EQUAL(num_pages, linkedlist_size(r->pages));
}

void test_resize_region_adjust_size_different_from_allocated_size_when_size_is_unaligned()
{
    const char* name = "test-region";
    const intptr_t new_size = SYSTEM_PAGE_SIZE + 20;
    memory_t* m = memory_management_create();

    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, name, 0, 0, true, true, true);
    TEST_ASSERT_NOT_NULL(r);

    bool success = memory_management_region_resize(r, new_size);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL_HEX(new_size, r->size);
    TEST_ASSERT_EQUAL_HEX(2*SYSTEM_PAGE_SIZE, r->allocated_size);
    TEST_ASSERT_EQUAL(2, linkedlist_size(r->pages));
}

void test_resize_region_doesnt_release_allocated_pages()
{
    const char* name = "test-region";
    const size_t num_pages = 20;
    const intptr_t size = num_pages*SYSTEM_PAGE_SIZE;
    memory_t* m = memory_management_create();

    TEST_ASSERT_NOT_NULL(m);
    // alocates double size
    memory_region_t* r = memory_management_region_create(m, name, 0, 2*size, true, true, true);
    TEST_ASSERT_NOT_NULL(r);

    TEST_ASSERT_EQUAL_HEX(2*size, r->allocated_size);
    TEST_ASSERT_EQUAL_HEX(2*size, r->size);

    bool success = memory_management_region_resize(r, size);
    TEST_ASSERT_TRUE(success);
    TEST_ASSERT_EQUAL_HEX(size, r->size);
    TEST_ASSERT_EQUAL_HEX(2*size, r->allocated_size);
    TEST_ASSERT_EQUAL(2*num_pages, linkedlist_size(r->pages));
}


static void map_physical_address_with_initial_size(size_t initial_size)
{
    const char* name = "test-region";
    const uintptr_t physical_start = 0x100000;
    const uintptr_t region_start_address = 0x40000;
    const size_t num_pages = 3;
    const intptr_t p_size = num_pages * SYSTEM_PAGE_SIZE;
    memory_t* m = memory_management_create();

    TEST_ASSERT_NOT_NULL(m);
    // alocates double size
    memory_region_t* r = memory_management_region_create(m, name, region_start_address, initial_size, true, true, true);
    TEST_ASSERT_NOT_NULL(r);

    uintptr_t vaddr = memory_management_region_map_physical_address(r, physical_start, p_size);

    // since there was 3 pages before the begin address is start + initial_size
    TEST_ASSERT_EQUAL(region_start_address + initial_size, vaddr);
    TEST_ASSERT_EQUAL_HEX(initial_size + p_size, r->allocated_size);
    TEST_ASSERT_EQUAL_HEX(r->allocated_size, r->size);
    TEST_ASSERT_EQUAL((initial_size + p_size)/SYSTEM_PAGE_SIZE, linkedlist_size(r->pages));
    
    // ensure pages are in the page list
    int page_index_start = linkedlist_size(r->pages) - num_pages;
    for (size_t i=0; i < num_pages; ++i) {
        TEST_ASSERT_EQUAL_HEX(physical_start + i * SYSTEM_PAGE_SIZE, (uintptr_t) linkedlist_get(r->pages, page_index_start + i));
    }
}

void test_map_physical_address_with_empty_region()
{
    map_physical_address_with_initial_size(0);
}

void test_map_physical_address_with_non_empty_region()
{
    map_physical_address_with_initial_size(5 * SYSTEM_PAGE_SIZE);
}

void test_get_physical_address()
{
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    const uintptr_t test_addr = 0x40000;
    memory_region_t* r = memory_management_region_create(m, "test", test_addr, 10, true, true, true);
    TEST_ASSERT_EQUAL_HEX(last_page_allocated, memory_management_get_physical_address(m, test_addr));
    TEST_ASSERT_EQUAL_HEX(last_page_allocated+5, memory_management_get_physical_address(m, test_addr+5));
}

void test_get_kernel_memory()
{
    memory_management_initialize();
    TEST_ASSERT_NOT_NULL(memory_management_get_kernel());
}

void test_get_size()
{
    const char* name = "test-region";
    const intptr_t start_addr = 0x404000;
    const intptr_t size = 10;
    memory_t* m = memory_management_create();
    TEST_ASSERT_NOT_NULL(m);
    memory_region_t* r = memory_management_region_create(m, name, start_addr, size, true, true, true);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_EQUAL(size, memory_management_region_current_size(r));
}
