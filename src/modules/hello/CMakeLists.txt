cmake_minimum_required (VERSION 3.7)
project (hello
    VERSION 0.9
    LANGUAGES NONE
)

enable_language(C ASM)

include(../common/CMakeListsModule.txt)

# TODO Pending build a gcc for my own OS

# add_library(crt0.o OBJECT "${CMAKE_CURRENT_LIST_DIR}/crt0.S")
# add_executable(hello.mod $<TARGET_OBJECTS:crt0.o>)

add_executable(hello.mod "")

target_sources(hello.mod
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/hello.c"
        "${CMAKE_CURRENT_LIST_DIR}/crt0.S"
)

target_link_libraries(hello.mod c)
