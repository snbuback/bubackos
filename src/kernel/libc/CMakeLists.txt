file(GLOB_RECURSE LIBC_SOURCES FOLLOW_SYMLINKS src/*.c)
include_directories("include")

add_library (c STATIC ${LIBC_SOURCES})
