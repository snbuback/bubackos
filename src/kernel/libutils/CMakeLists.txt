file(GLOB_RECURSE UTILS_SOURCES FOLLOW_SYMLINKS src/*.c)
include_directories("include")

add_library (utils STATIC ${UTILS_SOURCES})
