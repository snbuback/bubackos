#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_
#include <stdlib.h>
#include <stdbool.h>

// TODO this shouldn't be here
void* kalloc(size_t);
void kfree(void*);

#ifndef MEM_ALLOC
#define MEM_ALLOC(size)        kalloc(size)
#endif

#ifndef MEM_FREE
#define MEM_FREE(ptr)          kfree(ptr)
#endif

#define WHILE_LINKEDLIST_ITER(ll, type, var)    linkedlist_iter_t iter ## __LINE__; type var; linkedlist_iter_initialize(ll, &iter ## __LINE__); while ((var = (type) linkedlist_iter_next(&iter ## __LINE__)))

typedef struct linkedlist_node_t {
    void *val;
    struct linkedlist_node_t *next;
} linkedlist_node_t;

typedef struct {
    linkedlist_node_t* first;
    size_t size;
} linkedlist_t;

typedef struct {
    linkedlist_node_t* next_node;
} linkedlist_iter_t;

/**
 * Creates a new linked list
 */
linkedlist_t* linkedlist_create();

/**
 * Destroy a linked list, releasing all node (but not releasing pointer to data)
 * Complexity: O(N)
 */
void linkedlist_destroy(linkedlist_t* ll);

/**
 * Append the data to the end of list
 * Complexity: O(N)
 */
bool linkedlist_append(linkedlist_t* ll, void* const data);

/**
 * Remove the last element of a list. Returns its address or NULL if list if empty.
 * Complexity: O(N)
 */
void* linkedlist_pop(linkedlist_t* ll);

/**
 * Returns the i-index element of the list
 * Complexity: O(index)
 */
void* linkedlist_get(linkedlist_t* ll, size_t index);

/**
 * Returns the index of the element on the linkedlist. -1 if the element doesn't exist.
 * Complexity: O(n)
 */
int linkedlist_find(linkedlist_t* ll, void* el);

/**
 * Returns the size of the list
 * Complexity: O(1)
 */
size_t linkedlist_size(linkedlist_t* ll);

/**
 * Initialize an iterator (should be allocate on your stack)
 */
bool linkedlist_iter_initialize(linkedlist_t* ll, linkedlist_iter_t* iter);

/**
 * Iterates over the linkedlist
 * Complexity: O(1) for the next element
 */
void* linkedlist_iter_next(linkedlist_iter_t* iter);

/**
 * Remove the element with pointer to addr
 * Returns if the element was found and deleted.
 */
bool linkedlist_remove_element(linkedlist_t* ll, const void* addr);

#endif