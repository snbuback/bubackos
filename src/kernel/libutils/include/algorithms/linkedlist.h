#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_
#include <stdlib.h>
#include <stdbool.h>

#ifndef MEM_ALLOC
#define MEM_ALLOC(size)        kmem_alloc(size)
#define MEM_FREE(ptr)          kmem_free(ptr)
#endif

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

#endif