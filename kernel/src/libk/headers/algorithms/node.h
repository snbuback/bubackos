/*
 *  node.h
 *
 *  A polymorphic Linked-list Node Structure
 *
 */
#ifndef ALGORITHM_NODE_H
#define ALGORITHM_NODE_H
#include <core/memory.h>

#define LIBK_ALLOC(size)        kmem_alloc(size)
#define LIBK_FREE(ptr)          kmem_free(ptr)

typedef struct Node {
    void *val;
    struct Node *next;
} Node;

Node* list_new(void *val);
Node* list_append(Node *node, Node *new_node);

#endif