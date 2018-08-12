// mock implementations
#include <kernel_test.h>
#include <stdarg.h>

void logging(int level, const char *fmt, ...)
{
    printf(">>>%d: ", level);
    va_list args;
    va_start(args, fmt);

    int size = vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);  // disable buffering
}
