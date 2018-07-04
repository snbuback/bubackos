#ifndef _CORE_UTILS_H
#define _CORE_UTILS_H

#define NEW(type)				((type*) kmem_alloc(sizeof(type)))
#define FREE(addr)			kmem_free(addr)

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define ALIGN(addr, a)			(addr & ~(a - 1))

#endif

