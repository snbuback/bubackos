#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/config.h>
#include <sys/stat.h>
#include <kernel/page_allocator.h>
#include <kernel/console.h>
#include <kernel/logging.h>

// #define DEBUG_MEMORY
#pragma GCC diagnostic ignored "-Wunused-parameter"

static uint8_t mem[500*1024];
size_t pos = 0;

void * sbrk (ptrdiff_t incr)
{
#ifdef DEBUG_MEMORY
  char num_as_hex[50];  // Maximum number as string
  console_print("Allocated ");
  utoa((unsigned int) incr, num_as_hex, 10);
  console_print(num_as_hex);
#endif

  pos += incr;
  uintptr_t addr = (uintptr_t) (mem + pos);

#ifdef DEBUG_MEMORY
  console_print(" bytes at 0x");
  utoa((unsigned int) addr, num_as_hex, 16);
  console_print(num_as_hex);
  console_print(".\n");
#endif
  return (void*) addr;
}

int close (int fd)
{
	log_warn("close called");
  return 0;
}

_READ_WRITE_RETURN_TYPE write (int fd, const void *buf, size_t cnt)
{
  if (fd < 3) {
    console_write(buf, cnt);
    return cnt;
  }
	log_warn("write called");
  return 0;
}

_READ_WRITE_RETURN_TYPE read (int fd, void *buf, size_t cnt)
{
  log_warn("read called");
  return 0;
}

off_t lseek (int fd, off_t pos, int whence)
{
	log_warn("lseek called");
  return 0;
}

int fstat (int fd, struct stat *pstat)
{
	//log_warn("fstat called");  ERROR if logging is enabled
  return 0;
}

int isatty (int fd) {
    return fd < 3 ? 1 : 0;
}

// required by jerryscript
int gettimeofday(void) {
	log_warn("gettimeofday called");
	return 0;
}

void _exit(void) {
	log_warn("_exit called");
}

int error_no = 0;
int* __errno(void) {
	log_warn("__errno called");
	return &error_no;
}


