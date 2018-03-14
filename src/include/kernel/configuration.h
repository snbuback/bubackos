
// Defines the size of the pages of memory.
// The hardware implementation must support at least a multiple of this (ideally this size)
#define SYSTEM_PAGE_ALIGN  11
#define SYSTEM_PAGE_SIZE   (2 << SYSTEM_PAGE_ALIGN)
#define SYSTEM_LIMIT_OF_TASKS   1024
#define SYSTEM_DEBUG
