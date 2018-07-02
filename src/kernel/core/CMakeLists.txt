file(GLOB_RECURSE LIBCORE_SOURCES FOLLOW_SYMLINKS src/*.c)
include_directories("include")

add_library (core STATIC ${LIBCORE_SOURCES})
