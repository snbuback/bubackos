/*****************************************************************************
 **
 ** queue.h
 ** 
 ** Structure and interface definitions for Queue, a polymorphic FIFO
 ** data structure
 ** 
 ** from: https://github.com/raideus/c-data-structures
 **
 ****************************************************************************/


#include <algorithms/node.h>

#ifndef ALGORITHM_QUEUE_H
#define ALGORITHM_QUEUE_H

typedef struct Queue {
    unsigned size;
    Node *head;
    Node *tail;
} Queue;

extern Queue* Queue_new();
extern void Queue_add(Queue *q, void *el);
extern void* Queue_remove(Queue *q);
extern void* Queue_front(Queue *q);
extern unsigned Queue_size(Queue *q);
extern void Queue_delete(Queue *q);

#endif