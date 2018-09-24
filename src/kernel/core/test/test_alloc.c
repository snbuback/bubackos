// source: src/alloc.c
// source: src/vmem/services.c
// source: src/vmem/region_services.c
// source: ../libutils/src/libutils/id_mapper.c
// source: ../libutils/src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <string.h>
#include <stdbool.h>
#include <core/vmem/services.h>
#include <core/alloc.h>
#include <core/hal/platform.h>
#include <libutils/utils.h>

extern vmem_region_t* kernel_data_vmem_region;

// simulates the page allocation using the memory mapped
static uintptr_t last_page_allocated = -1;
uintptr_t pmem_allocate()
{
    last_page_allocated += SYSTEM_PAGE_SIZE;
    log_debug("Allocated page at %p", last_page_allocated);
    return last_page_allocated;
}

bool pmem_set_as_system(uintptr_t addr, size_t total_in_bytes)
{
    log_debug("Requested mark page %p as allocated. Current ptr=%p", addr, last_page_allocated);
    TEST_ASSERT_EQUAL_MESSAGE(ALIGN(addr, SYSTEM_PAGE_SIZE), addr, "page request should be aligned");
    if (last_page_allocated < addr) {
        last_page_allocated = addr;
    }
    return true;
}

void setUp(void)
{
    kernel_data_vmem_region = NULL;
    platform_t* platform = get_platform_config();
    last_page_allocated = ALIGN_NEXT(platform->memory.kernel_data.addr_end, SYSTEM_PAGE_SIZE);
    vmem_initialize();
    vmem_region_initialize();
    memory_allocator_initialize();
}

void tearDown(void)
{
    extern bool mem_allocator_initialised;
    mem_allocator_initialised = false;
}

// tests
void test_memory_allocator_creates_a_new_memory_region()
{
    // find the region kernel-data
    WHILE_LINKEDLIST_ITER(vmem_get_kernel()->regions, vmem_region_t*, region) {
        log_info("\n---> %s\n", region->name);
        if (strcmp("kernel-data", region->name) == 0) {
            break;
        }
    }

    TEST_ASSERT_NOT_NULL(region);
    platform_t* platform = get_platform_config();
    TEST_ASSERT_EQUAL_HEX(platform->memory.kernel_data.addr_start, region->start);
    TEST_ASSERT_EQUAL_HEX(platform->memory.kernel_data.addr_end, region->start + platform->memory.kernel_data.size);
}

void test_memory_allocator_allocates_memory_in_sequence()
{
    const size_t new_allocation = 20;
    uintptr_t initial_addr = (uintptr_t) kalloc(new_allocation);
    TEST_ASSERT_NOT_EQUAL(0, initial_addr);
    uintptr_t next_addr = (uintptr_t) kalloc(1);
    TEST_ASSERT_TRUE(next_addr >= initial_addr + new_allocation); // at least new_allocation
}

void test_memory_area_is_resized()
{
    // allocating the allocated size + 1 to ensure should be a expansion of kernel data memory
    const size_t mem_region_size = kernel_data_vmem_region->allocated_size;
    const size_t to_allocate = ALIGN_NEXT(mem_region_size + 1, SYSTEM_PAGE_SIZE);
    log_info("original size=%d", mem_region_size);
    uintptr_t initial_addr = (uintptr_t) kalloc(to_allocate);
    TEST_ASSERT_NOT_EQUAL(0, initial_addr);
    uintptr_t next_addr = (uintptr_t) kalloc(1);
    TEST_ASSERT_NOT_EQUAL(0, next_addr);
    log_info("next_addr=%p initial_addr=%p  difference=%d", next_addr, initial_addr, next_addr - initial_addr);
    log_info("size=%d allocated=%d", kernel_data_vmem_region->size, kernel_data_vmem_region->allocated_size);
    TEST_ASSERT_TRUE(kernel_data_vmem_region->allocated_size > mem_region_size);
    TEST_ASSERT_TRUE(next_addr > initial_addr); // at least new_allocation
}