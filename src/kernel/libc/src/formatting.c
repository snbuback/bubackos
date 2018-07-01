#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

static const char* NULL_PTR = "(null)";
static const char representation[] = "0123456789ABCDEF";
static int convert(unsigned long num, int base, char* dst, size_t size)
{
    size_t p = 0; // cursor position in buffer
    char buf[sizeof(num)*8]; // maximum number is on base 2

    if (size == 0) {
        return 0;
    }

    do
    {
        buf[p++] = representation[num%base];
        num /= base;
    } while(num != 0);

    size_t i;
    for (i=0; i<p && i<size; i++) {
        dst[i] = buf[p-i-1];
    }
    return i;
}

int snprintf(char * buffer, size_t buffer_size, const char * format, ...) {
    va_list args;
    va_start(args, format);
    int size = vsnprintf(buffer, buffer_size, format, args);
    va_end(args);
    return size;
}

int vsnprintf(char * buffer, size_t buffer_size, const char * format, va_list arg)
{
    #define WRITE_CHAR(c)       { buffer[p++] = c; }
    #define WRITE_SEQUENCE(s)   { size_t n = MIN(strlen(s), buffer_size-p); memcpy(buffer+p, s, n); p += n; }
    #define WRITE_NUMBER(n, base)   { p += convert(n, base, buffer+p, buffer_size-p); }

    char *traverse;
    size_t p=0; // position in buffer

    // convertion arguments
    unsigned long arg_ulong;
    long arg_long;
    char arg_char;
    void* arg_ptr;

    for(traverse = (char*) format; *traverse != '\0' && p < buffer_size; traverse++)
    {
        while( *traverse != '%' && *traverse != 0 && p < buffer_size)
        {
            WRITE_CHAR(*traverse);
            traverse++;
        }

        if ( *traverse == 0 ) {
            break;
        }

        // last character was %
        traverse++;

        //Module 2: Fetching and executing arguments
        switch(*traverse)
        {
            case 'c' : arg_char = va_arg(arg, int);		//Fetch char argument
                        WRITE_CHAR(arg_char);
                        break;

            case 'd' : arg_long = va_arg(arg, int); 		//Fetch Decimal/Integer argument
                        if(arg_long < 0) {
                            WRITE_CHAR('-');
                            WRITE_NUMBER((unsigned long) -arg_long, 10);
                        } else {
                            WRITE_NUMBER((unsigned long) arg_long, 10);
                        }
                        break;

            case 'o': arg_ulong = va_arg(arg, unsigned int); //Fetch Octal representation
                        WRITE_NUMBER(arg_ulong, 8);
                        break;

            case 's': arg_ptr = va_arg(arg, char*); 		// Fetch string
                        WRITE_SEQUENCE(arg_ptr ? (char*) arg_ptr : NULL_PTR);
                        break;

            case 'p': arg_ptr = va_arg(arg, void*); //Fetch Hexadecimal representation
                        if (arg_ptr) {
                            WRITE_SEQUENCE("0x");
                            WRITE_NUMBER((uintptr_t) arg_ptr, 16);
                        } else {
                            WRITE_SEQUENCE(NULL_PTR);
                        }
                        break;

            case 'x': arg_ulong = va_arg(arg, unsigned int); //Fetch Hexadecimal representation
                        WRITE_NUMBER(arg_ulong, 16);
                        break;

            case '%':
                        WRITE_CHAR('%');
                        break;
        }
    }

    // printf("--->%d %d\n", p, buffer_size);
    if (p == buffer_size) {
        p--;
    }
    buffer[p] = 0;

    return p;
}

