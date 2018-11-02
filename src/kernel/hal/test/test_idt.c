// source: src/x86_64/lib/idt.c
// source: ../core/src/scheduler/services.c
// source: ../libutils/src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <x86_64/idt.h>
#include <core/hal/hw_events.h>

void parse_intel_pagefault_flag(pagefault_status_t* pf, unsigned int pf_flag);
void test_parse_pagefault_usermode_accessing_kernel_pages(void)
{
    pagefault_status_t pf;
    parse_intel_pagefault_flag(&pf, 0x5);
    TEST_ASSERT_EQUAL_HEX(1, pf.is_reference_valid);
    TEST_ASSERT_EQUAL_HEX(0, pf.no_execution_access);
    TEST_ASSERT_EQUAL_HEX(1, pf.no_reading_access);
    TEST_ASSERT_EQUAL_HEX(0, pf.no_writing_access);
}
