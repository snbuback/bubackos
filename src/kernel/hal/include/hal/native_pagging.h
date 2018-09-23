#ifndef _HAL_PAGGING_H
#define _HAL_PAGGING_H
#include <core/types.h>
#include <stdbool.h>
#include <core/hal/native_vmem.h>

typedef void (*entry_visited_func)(page_map_entry_t* entry);

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

void native_page_table_flush();

/**
 * Dump current page table
 * pt == NULL dumps active page table
 */
void native_pagetable_dump(native_page_table_t* pt);

#endif