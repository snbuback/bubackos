#include <algorithms/linkedlist.h>
#include <core/memory_management.h>

linkedlist_t* linkedlist_create() {
    linkedlist_t* linkedlist = (linkedlist_t*) MEM_ALLOC(sizeof(linkedlist_t));
    if (linkedlist == NULL) {
        // no memory available
        return NULL;
    }
    linkedlist->first = NULL;
    linkedlist->size = 0;
    return linkedlist;
}

size_t linkedlist_size(linkedlist_t* ll)
{
    if (!ll) {
        return 0;
    }
    return ll->size;
}

bool linkedlist_iter_initialize(linkedlist_t* ll, linkedlist_iter_t* iter)
{
    if (!ll) {
        return false;
    }
    if (!iter) {
        return false;
    }
    iter->next_node = ll->first;
    return true;
}

/**
 * Essentially this is the iterator but instead of return the value, return the node structure
 * used by many linkedlist functions
 */
static linkedlist_node_t* linkedlist_iter_next_node(linkedlist_iter_t* iter)
{
    if (!iter || !iter->next_node) {
        return NULL;
    }

    linkedlist_node_t* current = iter->next_node;
    iter->next_node = current->next;
    return current;
}

void* linkedlist_iter_next(linkedlist_iter_t* iter)
{
    linkedlist_node_t* current = linkedlist_iter_next_node(iter);
    if (!current) {
        return NULL;
    }
    return current->val;
}

void* linkedlist_get(linkedlist_t* ll, size_t index)
{
    if (!ll || index >= ll->size) {
        return NULL;
    }
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    linkedlist_node_t* next = linkedlist_iter_next_node(&iter);
    for (size_t i=0; i<index && next; ++i) {
        next = linkedlist_iter_next_node(&iter);
    }
    if (!next) {
        return NULL;
    }
    return next->val;
}

void linkedlist_destroy(linkedlist_t* ll)
{
    // walk over the list releasing all node
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    linkedlist_node_t* last = linkedlist_iter_next_node(&iter);
    linkedlist_node_t* next = linkedlist_iter_next_node(&iter);
    while ((next = linkedlist_iter_next_node(&iter))) {
        MEM_FREE(last);
        last = next;
    }
    // release the latest element
    MEM_FREE(last);
    MEM_FREE(ll);
}

bool linkedlist_append(linkedlist_t* ll, void* const data)
{
    if (!ll) {
        // no list allocated
        return false;
    }

    linkedlist_node_t* node = (linkedlist_node_t*) MEM_ALLOC(sizeof(linkedlist_node_t));
    if (node == NULL) {
        return false;
    }
    node->val = data;
    node->next = NULL;
    if (ll->size == 0) {
        ll->first = node;
    } else {
        // iterate until the last element. Size the last have at least 1 element, last != NULL
        linkedlist_iter_t iter;
        linkedlist_iter_initialize(ll, &iter);
        linkedlist_node_t* last = NULL;
        linkedlist_node_t* next = NULL;
        while ((next = linkedlist_iter_next_node(&iter))) {
            last = next;
        }
        last->next = node;
    }
    ++ll->size;
    return true;
}

void* linkedlist_pop(linkedlist_t* ll)
{
    if (!ll || !ll->first) {
        // no list allocated
        return NULL;
    }

    linkedlist_node_t* last;
    if (ll->size == 1) {
        last = ll->first;
        ll->first = NULL;
    } else {
        // iterate until the last element. Size the last have at least 1 element, last != NULL
        linkedlist_iter_t iter;
        linkedlist_iter_initialize(ll, &iter);
        linkedlist_node_t* next = NULL;
        linkedlist_node_t* previous = NULL;
        while ((next = linkedlist_iter_next_node(&iter))) {
            previous = last;
            last = next;
        }
        previous->next = NULL; //  release the last
    }
    --ll->size;
    void* data = last->val;
    MEM_FREE(last);
    return data;
}

