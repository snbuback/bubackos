#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/config.h>
#include <sys/stat.h>
#include <kernel/page_allocator.h>
#include <kernel/console.h>
#include <kernel/logging.h>

static char mem[500*1024];
size_t pos = 0;

void * sbrk (ptrdiff_t incr)
{
#ifdef SYSTEM_DEBUG
  char num_as_hex[50];  // Maximum number as string
  terminal__print("Allocated ");
  utoa(incr, num_as_hex, 10);
  terminal__print(num_as_hex);
#endif

  pos += incr;
  void *addr = mem + pos;

#ifdef SYSTEM_DEBUG
  terminal__print(" bytes at 0x");
  utoa(addr, num_as_hex, 16);
  terminal__print(num_as_hex);
  terminal__print(".\n");
#endif
  return addr;
}

int close (int fd)
{
	LOG_WARNING("close called");
  return 0;
}

_READ_WRITE_RETURN_TYPE write (int fd, const void *buf, size_t cnt)
{
  if (fd < 3) {
    terminal__write(buf, cnt);
    return cnt;
  }
	LOG_WARNING("write called");
  return 0;
}

_READ_WRITE_RETURN_TYPE read (int fd, void *buf, size_t cnt)
{
  LOG_WARNING("read called");
  return 0;
}

off_t lseek (int fd, off_t pos, int whence)
{
	LOG_WARNING("lseek called");
  return 0;
}

int fstat (int fd, struct stat *pstat)
{
	//LOG_WARNING("fstat called");  ERROR if logging is enabled
  return 0;
}

int isatty (int fd) {
    return fd < 3 ? 1 : 0;
}

// required by jerryscript
int gettimeofday(void) {
	LOG_WARNING("gettimeofday called");
	return 0;
}

void _exit(void) {
	LOG_WARNING("_exit called");
}

int error_no = 0;
int* __errno(void) {
	LOG_WARNING("__errno called");
	return &error_no;
}


