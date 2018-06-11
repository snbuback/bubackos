// source: kernel/c/hal/x86_64/gdt.c
#include <kernel_test.h>
#include <hal/gdt.h>
#include <hal/tss.h>

#define GDT_ENTRY_BASE(i)       (((uint64_t) gdt_table[i].base_0_15) + ((uint64_t) gdt_table[i].base_16_23 << 16) + ((uint64_t) gdt_table[i].base_24_31 << 24) + ((uint64_t) gdt_table[i].base_32_63 << 32))
#define GDT_ENTRY_LIMIT(i)      (((uint64_t) gdt_table[i].limit_0_15) + ((uint64_t) gdt_table[i].limit_16_19 << 16))

static bool gdt_flush_called = false;
void gdt_flush(uintptr_t base, uint16_t limit)
{
    gdt_flush_called = true;

    // assert size
    TEST_ASSERT_EQUAL_UINT(sizeof(gdt_entry) * GDT_MAXIMUM_SIZE - 1, limit);

    gdt_entry* gdt_table = (gdt_entry*) base;

    // check segment tables
    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_BASE(0));
    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_LIMIT(0));

    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_BASE(1));
    TEST_ASSERT_EQUAL_HEX64(0xFFFFF, GDT_ENTRY_LIMIT(1));

    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_BASE(2));
    TEST_ASSERT_EQUAL_HEX64(0xFFFFF, GDT_ENTRY_LIMIT(2));

    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_BASE(3));
    TEST_ASSERT_EQUAL_HEX64(0xFFFFF, GDT_ENTRY_LIMIT(3));

    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_BASE(4));
    TEST_ASSERT_EQUAL_HEX64(0xFFFFF, GDT_ENTRY_LIMIT(4));

    // assert TSS
    tss_entry_t* tss_entry = (tss_entry_t*) GDT_ENTRY_BASE(5);
    // size of tss is 0 second intel manual: when the granularity flag is set, a limit of 0 results in valid offsets from 0 to 4095
    TEST_ASSERT_EQUAL_HEX64(0, GDT_ENTRY_LIMIT(5));

    // check kernel stack pointer
    uintptr_t* kstack = get_kernel_stack();
    TEST_ASSERT_EQUAL_HEX64(kstack[0], tss_entry->rsp0_0_31 + (((uintptr_t) tss_entry->rsp0_32_63) << 32) );

}

static bool tss_flush_called = false;
void tss_flush(uint16_t gdt_entry_number)
{
    tss_flush_called = true;
    TEST_ASSERT_EQUAL_HEX16(0x50, gdt_entry_number);
}

void setUp(void)
{
    gdt_flush_called = false;
    tss_flush_called = false;
}


// no format
void test_gdt_install(void)
{
    gdt_install();
    uintptr_t* kstack = get_kernel_stack();
    TEST_ASSERT_TRUE(tss_flush_called);
    TEST_ASSERT_TRUE(gdt_flush_called);
}
