#ifndef _LIBUTILS_ITER_H_
#define _LIBUTILS_ITER_H_

typedef void* (*next_iter_func)(void *iter_data);

typedef struct {
    next_iter_func next_func;
    void* iter_data;
} iter_t;

#define ITER_NEXT(iter)     iter.next_func(iter.iter_data)

#endif