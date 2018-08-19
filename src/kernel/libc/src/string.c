/*
 * Implementation of libc basic functions
 * from: https://git.musl-libc.org/cgit/musl/tree/src/string
 */
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/UCHAR_MAX)
#define HIGHS (ONES * (UCHAR_MAX/2+1))
#define HASZERO(x) (((x)-ONES) & ~(x) & HIGHS)

size_t strlen(const char *s)
{
    const char *a = s;
    const size_t *w;
    for (; (uintptr_t)s % ALIGN; s++) if (!*s) return s-a;
    for (w = (const void *)s; !HASZERO(*w); w++);
    for (s = (const void *)w; *s; s++);
    return s-a;
}

char *strncpy(char *restrict d, const char *restrict s, size_t n)
{
    size_t *wd;
    const size_t *ws;

    if (((uintptr_t)s & ALIGN) == ((uintptr_t)d & ALIGN)) {
        for (; ((uintptr_t)s & ALIGN) && n && (*d=*s); n--, s++, d++);
        if (!n || !*s) goto tail;
        wd=(void *)d; ws=(const void *)s;
        for (; n>=sizeof(size_t) && !HASZERO(*ws);
               n-=sizeof(size_t), ws++, wd++) *wd = *ws;
        d=(void *)wd; s=(const void *)ws;
    }
    for (; n && (*d=*s); n--, s++, d++);
tail:
    memset(d, 0, n);
    return d;
}

int strncmp(const char *_l, const char *_r, size_t n)
{
  const unsigned char *l=(void *)_l, *r=(void *)_r;
  if (!n--) return 0;
  for (; *l && *r && n && *l == *r ; l++, r++, n--);
  return *l - *r;
}

void * memset (void *dest, int val, size_t len)
{
  unsigned char *ptr = dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

void * memcpy (void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}
