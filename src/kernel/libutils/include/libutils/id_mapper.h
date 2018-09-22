#ifndef _LIBUTILS_ID_MAPPER_H_
#define _LIBUTILS_ID_MAPPER_H_
#include <stdatomic.h>
#include <algorithms/linkedlist.h>

// make the type of id_handler_t in sync with id_mapper_t.next
typedef unsigned long id_handler_t;

typedef struct {
    // using atomic types
    atomic_ulong next;
    linkedlist_t* list; // of id_mapper_entry_t
} id_mapper_t;

typedef struct {
    id_handler_t id;
    void* val;
} id_mapper_entry_t;

/**
 * Initialise the id mapper struct.
 * @param id_mapper pointer to struct to initialise or NULL to allocate a new address in memory.
 * @return address of id_mapper_t or NULL if case of failure.
 */
id_mapper_t* id_mapper_create(id_mapper_t* id_mapper);

/**
 * Returns the next id. This function increments the counter.
 * This is used internally.
 */
id_handler_t id_mapper_next_id(id_mapper_t* id_mapper);

/**
 * Adds a new entry in the id_mapper. Returns the id of the entry.
 */
id_handler_t id_mapper_add(id_mapper_t* id_mapper, void* val);

/**
 * Returns the object associate to a given id.
 */
void* id_mapper_get(id_mapper_t* id_mapper, id_handler_t id);

/**
 * Removes one id entry
 */
bool id_mapper_del(id_mapper_t* id_mapper, id_handler_t id);

/**
 * Iterates over all elements.
 * TODO Change to use a more generic approach.
 */
linkedlist_iter_t id_mapper_iter(id_mapper_t* id_mapper);

/**
 * Release all the memory allocated by the id_mapper.
 * Only used when id_mapper is dinamically allocated.
 * TODO Create a is_dinamically field in the struct to mark if this call is required or not.
 */
void id_mapper_destroy(id_mapper_t* id_mapper);

#endif