cmake_minimum_required (VERSION 3.7)
project (hello
    VERSION 0.9
    LANGUAGES NONE
)

enable_language(C ASM)

include(../common/CMakeListsModule.txt)

add_executable(hello.mod "")

target_sources(hello.mod
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/hello.c"
        "${CMAKE_CURRENT_LIST_DIR}/userspace_init.S"
)
