#ifndef _CORE_HAL_HW_EVENTS_H_
#define _CORE_HAL_HW_EVENTS_H_
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uintptr_t addr;
    uintptr_t codeptr;
    uintptr_t stackptr;
    bool no_execution_access;
    bool no_reading_access;
    bool no_writing_access;
    bool is_reference_valid;
} pagefault_status_t;

//---------- implemented by Core ----------//
// https://wiki.osdev.org/Exceptions
void handle_page_fault(pagefault_status_t pf);

void handle_double_fault();

void handle_divide_by_zero();

void handle_debug();

void handle_invalid_operation();

void handle_protection_fault();

long handle_syscall(unsigned long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5);

void handle_generic_hw_events();

#endif