#include <stddef.h>
#include <stdint.h>
#include <sys/config.h>
#include <sys/stat.h>
#include <kernel/page_allocator.h>
#include <kernel/console.h>
#include <kernel/logging.h>

static char mem[500*1024];
size_t pos = 0;

void * sbrk (ptrdiff_t incr)
{
  pos += incr;
  terminal__write("Allocated memory\n", 18);
  // LOG_DEBUG("Allocated %d bytes at 0x%x (%d)", incr, &mem[pos], pos);
  return mem + pos;
  // return page_allocator_allocate();
}

int close (int fd)
{
  return 0;
}

_READ_WRITE_RETURN_TYPE write (int fd, const void *buf, size_t cnt)
{
  if (fd < 3) {
    terminal__write(buf, cnt);
    return cnt;
  }
  return 0;
}

_READ_WRITE_RETURN_TYPE read (int fd, void *buf, size_t cnt)
{
  return 0;
}

off_t lseek (int fd, off_t pos, int whence)
{
  return 0;
}

int fstat (int fd, struct stat *pstat)
{
  return 0;
}

int isatty (int fd) {
    return fd < 3 ? 1 : 0;
}


