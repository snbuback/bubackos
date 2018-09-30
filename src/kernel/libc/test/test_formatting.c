// source: src/formatting.c
#include <unity.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define BUFFER_SIZE     10000
char buffer[BUFFER_SIZE];

void setUp(void)
{
    memset(buffer, '@', BUFFER_SIZE); // fill buffer with @ (!= 0) to avoid hide non-null-termination bugs
}

size_t call_fmt(size_t buffer_size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t result = vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
    return result;
}

#define VALIDATE_FORMATTING(expected, fmt, fmt_args...)   VALIDATE_NFORMATTING(expected, BUFFER_SIZE, fmt, ##fmt_args)

#define VALIDATE_NFORMATTING(expected, sz, fmt, fmt_args...)   { \
    const char* str = expected; size_t str_sz = strlen(str); \
    size_t result = call_fmt(sz, fmt, ##fmt_args); \
    TEST_ASSERT_EQUAL_STRING(str, buffer); \
    TEST_ASSERT_EQUAL_UINT(str_sz, result); \
};

// no format
void test_no_format_empty_string(void)
{
    VALIDATE_FORMATTING("", "");
}

void test_no_format_one_char(void)
{
    VALIDATE_FORMATTING("w", "w");
}

void test_no_format_simple_string(void)
{
    VALIDATE_FORMATTING("hello word", "hello word");
}

void test_no_format_simple_string_with_buffer_overflow(void)
{

    VALIDATE_NFORMATTING("my strin", 9, "my string");
}

// escaping
void test_escape(void)
{
    VALIDATE_FORMATTING("%", "%%");
}

// number tests
void test_positive_decimal(void)
{
    VALIDATE_FORMATTING("hello word 345678.", "hello word %d.", (int) 345678);
}

void test_negative_decimal(void)
{
    VALIDATE_FORMATTING("hello word -345678.", "hello word %d.", (int) -345678);
}

void test_positive_long(void)
{
    VALIDATE_FORMATTING("hello word 345678.", "hello word %d.", (long) 345678);
}

void test_negative_long(void)
{
    VALIDATE_FORMATTING("hello word -345678.", "hello word %d.", (long) -345678);
}

void test_octal(void)
{
    VALIDATE_FORMATTING("23165", "%o", 9845);
}

void test_hex(void)
{
    VALIDATE_FORMATTING("2675", "%x", 9845);
}

void test_neg_number_with_overflow(void) {
    VALIDATE_NFORMATTING("d=-1", 5, "d=%d", -123456789);
}

void test_number_with_overflow(void) {
    VALIDATE_NFORMATTING("d=12", 5, "d=%d", 123456789);
}

// pointer tests
void test_null_pointer(void)
{
    void* ptr = NULL;
    VALIDATE_FORMATTING("ptr=(null).", "ptr=%p.", ptr);
}

void test_pointer(void)
{
    void* ptr = (void*) 0x10002;
    VALIDATE_FORMATTING("ptr=0x10002.", "ptr=%p.", ptr);
}

// char/string tests
void test_char(void)
{
    VALIDATE_FORMATTING(">A<", ">%c<", 'A');
}

void test_high_char(void)
{
    VALIDATE_FORMATTING(">\xC8<", ">%c<", 200);
}

void test_null_string(void)
{
    char* ptr = NULL;
    VALIDATE_FORMATTING("str=(null).", "str=%s.", ptr);
}

void test_null_string_with_overflow(void)
{
    char* ptr = NULL;
    VALIDATE_NFORMATTING("str=(n", 7, "str=%s.", ptr);
}

void test_string(void)
{
    char* ptr = "my string";
    VALIDATE_FORMATTING("str=my string.", "str=%s.", ptr);
}

void test_string_with_overflow(void)
{
    char* ptr = "my_string";
    VALIDATE_NFORMATTING("str=my_s", 9, "str=%s.", ptr);
}

