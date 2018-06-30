cmake_minimum_required (VERSION 3.7)
project (metaOS-core VERSION 0.9)
enable_language(C ASM)
set(CMAKE_VERBOSE_MAKEFILE on)

string(APPEND CMAKE_C_FLAGS "${FLAGS_COMMON_ARCH} -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -include hal/debugger.h")

# TODO Move append of "-include", "hal/debugger.h" to Debug profile
string(APPEND CMAKE_ASM_FLAGS "${FLAGS_COMMON_ARCH} -Wall -Werror -Wextra -DNDEBUG")
string(APPEND CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-n,-L/usr/local/x86_64-elf/lib -T ${CMAKE_SOURCE_DIR}/src/kernel/c/hal/x86_64/linker.ld")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/../build/bootloader/boot")

file(GLOB_RECURSE CORE_SOURCES FOLLOW_SYMLINKS src/kernel/c/*.c src/kernel/c/*.S)
include_directories("src/kernel/headers")

file(GLOB_RECURSE LIBK_SOURCES FOLLOW_SYMLINKS src/libk/c/*.c)
include_directories("src/libk/headers")

set(SOURCES
    ${CORE_SOURCES}
    ${LIBK_SOURCES}
)
add_executable(kernel.elf ${SOURCES})
