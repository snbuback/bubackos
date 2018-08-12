// Defines the size of the pages of memory.
// The hardware implementation must support at least a multiple of this (ideally this size)
#define SYSTEM_PAGE_ALIGN  12
#define SYSTEM_PAGE_SIZE   (2 << (SYSTEM_PAGE_ALIGN-1))
#define SYSTEM_LIMIT_OF_TASKS   10
#define SYSTEM_LIMIT_OF_MEMORY_HANDLER   10
#define SYSTEM_DEBUG
#define SYSTEM_STACKSIZE    32768

// debug kernel memory allocation
// #define SYSTEM_DEBUG_MEMORY

// TODO Add in the configuration an include per platform
