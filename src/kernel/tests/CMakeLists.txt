add_library(unity "")

target_sources(unity
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/unity/src/unity.c"
        "${CMAKE_CURRENT_LIST_DIR}/unity/src/unity_internals.h"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/unity/src/unity.h"
)

target_include_directories(unity PUBLIC "${CMAKE_CURRENT_LIST_DIR}/unity/src")
# set_target_properties(unity PROPERTIES CMAKE_C_COMPILER ${CMAKE_C_TEST_COMPILER})
