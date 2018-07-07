cmake_minimum_required (VERSION 3.7)
project (hello-module)
enable_language(C ASM)
set(CMAKE_VERBOSE_MAKEFILE on)
string(APPEND CMAKE_C_FLAGS "${FLAGS_COMMON_ARCH} -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG")
string(APPEND CMAKE_ASM_FLAGS "${FLAGS_COMMON_ARCH} -Wall -Werror -Wextra -DNDEBUG")
string(APPEND CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,-z,common-page-size=4096,-z,max-page-size=4096 -T ${CMAKE_SOURCE_DIR}/module.ld")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/../../build/bootloader/boot/")

set(SOURCES
    userspace_init.S
    hello.c
)
add_executable(hello.mod ${SOURCES})
