cmake_minimum_required (VERSION 3.7)
project (metaOS-core VERSION 0.9)
enable_language(C ASM)
set(CMAKE_VERBOSE_MAKEFILE on)

string(APPEND CMAKE_C_FLAGS "${FLAGS_COMMON_ARCH} -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -include hal/debugger.h")

# TODO Move append of "-include", "hal/debugger.h" to Debug profile
string(APPEND CMAKE_ASM_FLAGS "${FLAGS_COMMON_ARCH} -Wall -Werror -Wextra -DNDEBUG")
string(APPEND CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-n,-L/usr/local/x86_64-elf/lib -T ${CMAKE_SOURCE_DIR}/kernel/hal/src/x86_64/linker.ld")

# set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/..build/bootloader/boot")

file(GLOB_RECURSE CORE_SOURCES FOLLOW_SYMLINKS kernel/core/src/*.c)
include_directories("${CMAKE_SOURCE_DIR}/kernel/core/include")

file(GLOB_RECURSE LIBC_SOURCES FOLLOW_SYMLINKS kernel/libc/src/*.c)
include_directories("${CMAKE_SOURCE_DIR}/kernel/libc/include")

file(GLOB_RECURSE HAL_SOURCES FOLLOW_SYMLINKS kernel/hal/src/*.c kernel/hal/src/*.S)
include_directories("${CMAKE_SOURCE_DIR}/kernel/hal/include")

file(GLOB_RECURSE LIBU_SOURCES FOLLOW_SYMLINKS kernel/libutils/src/*.c)
include_directories("${CMAKE_SOURCE_DIR}/kernel/libutils/include")

set(SOURCES
    ${CORE_SOURCES}
    ${LIBC_SOURCES}
    ${HAL_SOURCES}
    ${LIBU_SOURCES}
)

add_executable(kernel.elf ${SOURCES})
