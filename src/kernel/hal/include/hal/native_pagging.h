#ifndef _HAL_PAGGING_H
#define _HAL_PAGGING_H
#include <core/types.h>

typedef struct {
    uintptr_t virtual_addr;
    uintptr_t physical_addr;
    size_t size;
    permission_t permission;
    bool present;
} page_map_entry_t;

#include <x86_64/native_pagging.h>

// alocate memory to the native page structure
native_page_table_t* native_pagetable_create();

/**
 * Add a new memory mapping to the native page structure.
 * This function is called with one 1 page.
 */
void native_pagetable_set(native_page_table_t* pt, page_map_entry_t entry);

/**
 * Switches the current cpu to the native_page_table_t
 */
void native_pagetable_switch(native_page_table_t* pt);

#endif