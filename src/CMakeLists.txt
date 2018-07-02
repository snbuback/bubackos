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

file(GLOB_RECURSE HAL_SOURCES FOLLOW_SYMLINKS kernel/hal/src/*.c kernel/hal/src/*.S)
include_directories("${CMAKE_SOURCE_DIR}/kernel/hal/include")

set(SOURCES
    ${CORE_SOURCES}
    ${LIBC_SOURCES}
    ${HAL_SOURCES}
)

add_executable(kernel.elf ${SOURCES})

# libc
add_subdirectory(kernel/libc)
include_directories("kernel/libc/include")
target_link_libraries(kernel.elf PUBLIC c)

# libutils
add_subdirectory(kernel/libutils)
include_directories("kernel/libutils/include")
target_link_libraries(kernel.elf PUBLIC utils)

