cmake_minimum_required (VERSION 3.7)
project (metaOS-core 
    VERSION 0.9
    LANGUAGES NONE    
)

enable_language(C ASM)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_CROSSCOMPILING on)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS_COMMON_ARCH} -fPIC -g -O0 -ffreestanding -Wall -Werror -Wextra -fdiagnostics-color=always -include x86_64/debugger.h")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${FLAGS_COMMON_ARCH} -g -O0 -Wall -Werror -Wextra -fdiagnostics-color=always -include x86_64/debugger.h")
set(UNITY_ROOT "${CMAKE_SOURCE_DIR}/kernel/tests/unity")

include(CMakeListsUtils.txt)

set(OS_EXEC_TARGET "kernel.elf")
add_executable(${OS_EXEC_TARGET} "")

# add_executable(kernel.elf "")

# libc
include(kernel/libc/CMakeLists.txt)

# libutils
include(kernel/libutils/CMakeLists.txt)

set_target_properties(${OS_EXEC_TARGET} PROPERTIES LINK_FLAGS "-fPIC -nostdlib -Wl,-n,-L/usr/local/x86_64-elf/lib -T ${CMAKE_SOURCE_DIR}/kernel/hal/src/x86_64/linker.ld")
target_link_libraries(${OS_EXEC_TARGET} PUBLIC utils)
target_link_libraries(${OS_EXEC_TARGET} PUBLIC c)

# # hal
include(kernel/hal/CMakeLists.txt)

# # libcore
include(kernel/core/CMakeLists.txt)

