#include <stddef.h>

// Defines the size of the pages of memory.
// The hardware implementation must support at least a multiple of this (ideally this size)
#define SYSTEM_PAGE_ALIGN  12
#define SYSTEM_PAGE_SIZE   (2 << (SYSTEM_PAGE_ALIGN-1))
#define SYSTEM_LIMIT_OF_TASKS   1024
#define SYSTEM_DEBUG

// debug kernel memory allocation
#define SYSTEM_DEBUG_MEMORY