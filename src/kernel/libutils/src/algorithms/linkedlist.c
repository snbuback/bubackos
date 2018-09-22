#include <algorithms/linkedlist.h>
#include <stdlib.h>

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


int linkedlist_find(linkedlist_t* ll, void* el)
{
    if (!ll) {
        return -1;
    }

    int i = 0;
    WHILE_LINKEDLIST_ITER(ll, void*, e) {
        if (e == el) {
            return i;
        }
        ++i;
    }
    return -1;
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

static void linkedlist_remove_node(linkedlist_t* ll, linkedlist_node_t* previous, linkedlist_node_t* removed)
{
    if (!removed) {
        return;
    }

    if (previous) {
        previous->next = removed->next;
    }

    if (ll->first == removed) {
        ll->first = removed->next;
    }

    MEM_FREE(removed);
    --ll->size;
}

bool linkedlist_remove_element(linkedlist_t* ll, const void* addr)
{
    if (!ll) {
        return false;
    }

    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    linkedlist_node_t* next = NULL;
    linkedlist_node_t* previous = NULL;
    while ((next = linkedlist_iter_next_node(&iter))) {
        if (next->val == addr) {
            // element to remove
            linkedlist_remove_node(ll, previous, next);
            return true;
        }
        previous = next;
    }
    return false;
}

int linkedlist_iter_remove_element(void* remaining, __attribute__((unused)) void* value)
{
    if (*(size_t*) remaining == 0) {
        return ITER_REMOVE_AND_RETURN;
    }
    --*(size_t*) remaining;
    return ITER_GO_NEXT;
}

void* linkedlist_remove(linkedlist_t* ll, size_t index)
{
    if (!ll || index >= ll->size) {
        // no list allocated or out of bounds
        return NULL;
    }

    void* val = linkedlist_iter_with_action(ll, linkedlist_iter_remove_element, &index);
    return val;
}

void* linkedlist_pop(linkedlist_t* ll)
{
    if (!ll || !ll->first) {
        // no list allocated
        return NULL;
    }
    return linkedlist_remove(ll, linkedlist_size(ll) - 1);
}

void* linkedlist_iter_with_action(linkedlist_t* ll, linkedlist_iter_action_func iter_func, void* data_to_func)
{
    void *val;
    linkedlist_iter_t iter;
    linkedlist_iter_initialize(ll, &iter);
    linkedlist_node_t* next = NULL;
    linkedlist_node_t* previous = NULL;
    while ((next = linkedlist_iter_next_node(&iter))) {
        switch (iter_func(data_to_func, next->val)) {
            case ITER_STOP_AND_RETURN:
                return next->val;

            case ITER_GO_NEXT:
                break;

            case ITER_REMOVE_AND_NEXT:
                linkedlist_remove_node(ll, previous, next);
                break;

            case ITER_REMOVE_AND_RETURN:
                val = next->val;
                linkedlist_remove_node(ll, previous, next);
                return val;

            default:
                // invalid action. Abort!
                return NULL;
        }
        previous = next;
    }
    return NULL;
}