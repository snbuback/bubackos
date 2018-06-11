// mock implementations
#include <kernel_test.h>

void logging(int level, const char *tag, const char *fmt, ...)
{
    printf(">>>%d %s:", level, tag);
    va_list args;
    va_start(args, fmt);

    int size = vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);  // disable buffering
}

void* kmem_alloc(size_t size)
{
    return malloc(size);
}

void kmem_free(void *ptr)
{
    free(ptr);
}
