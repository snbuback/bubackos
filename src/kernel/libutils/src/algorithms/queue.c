/*****************************************************************************
 **
 ** queue.c
 ** 
 ** Function implementations for Queue, a polymorphic FIFO data structure
 ** 
 ** Author: Sean Butze
 **
 ****************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <algorithms/queue.h>


Queue* Queue_new() {
    Queue *q = LIBK_ALLOC(sizeof(Queue));
    q->size = 0;
    q->head = NULL;
    q->tail = NULL;
    return q;
}

void Queue_add(Queue *q, void *el) {
    // Node *temp = q->head;
    Node *new_tail = LIBK_ALLOC(sizeof(Node));
    new_tail->val = el;
    new_tail->next = NULL;

    if (q->size < 1) {
        q->head = new_tail;
    } else {
        q->tail->next = new_tail;
    }

    q->tail = new_tail;
    q->size++;
}

void* Queue_front(Queue *q) {
    assert(q && q->size > 0);
    return (q->head->val);
}

void* Queue_remove(Queue *q) {
    assert(q && q->size > 0);
    Node *head = q->head;
    void *headval = head->val;
    q->head = head->next;

    if (q->size == 1) {
        q->tail = NULL; // Prevent dangling pointer
    }

    q->size--;
    LIBK_FREE(head);
    return (headval);
}

unsigned Queue_size(Queue *q) {
    assert(q);
    return(q->size);
}

void Queue_delete(Queue *q) {
    assert(q);
    Node *i1, *i2;

    for (i1 = q->head; i1; i1 = i2) {
        i2 = i1->next;
        LIBK_FREE(i1);
    }

    LIBK_FREE(q);
}