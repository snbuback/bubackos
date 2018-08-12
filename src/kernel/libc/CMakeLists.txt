add_library(c "")

target_sources(c
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/formatting.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/string.c"
)

target_add_test(c test_formatting ${CMAKE_CURRENT_LIST_DIR}/test/test_formatting.c)
