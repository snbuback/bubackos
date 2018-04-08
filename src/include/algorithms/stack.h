/*****************************************************************************
 **
 ** stack.h
 ** 
 ** Structure and interface definitions for Stack, a polymorphic LIFO
 ** data structure
 ** 
 ** from: https://github.com/raideus/c-data-structures
 **
 ****************************************************************************/


#include <algorithms/node.h>

#ifndef ALGORITHM_STACK_H
#define ALGORITHM_STACK_H

typedef struct Stack {
    unsigned size;
    Node *head;
} Stack;

extern Stack* Stack_new();
extern void Stack_push(Stack *s, void *el);
extern void* Stack_pop(Stack *s);
extern void* Stack_peek(Stack *s);
extern unsigned Stack_size(Stack *s);
extern void Stack_delete(Stack *s);

#endif