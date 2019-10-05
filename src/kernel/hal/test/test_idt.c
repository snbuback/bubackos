// source: src/x86_64/lib/idt.c
// source: ../core/src/scheduler/services.c
// source: ../libutils/src/algorithms/linkedlist.c
#include <kernel_test.h>
#include <x86_64/idt.h>
#include <core/hal/hw_events.h>

void parse_intel_pagefault_flag(pagefault_status_t* pf, uintptr_t pagefault_addr, native_task_t* native_task);

void test_parse_pagefault_usermode_accessing_kernel_pages(void)
{
    native_task_t ntask = {
        .orig_rax = 5
    };
    pagefault_status_t pf;
    parse_intel_pagefault_flag(&pf, 0x20, &ntask);
    TEST_ASSERT_EQUAL_HEX(1, pf.is_reference_valid);
    TEST_ASSERT_EQUAL_HEX(0, pf.no_execution_access);
    TEST_ASSERT_EQUAL_HEX(1, pf.no_reading_access);
    TEST_ASSERT_EQUAL_HEX(0, pf.no_writing_access);
}
