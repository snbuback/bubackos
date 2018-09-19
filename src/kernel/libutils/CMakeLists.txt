add_library(utils "")

target_sources(utils
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/libutils/id_mapper.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/algorithms/linkedlist.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/logging.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include/libutils/id_mapper.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/libutils/iter.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/libutils/utils.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/algorithms/linkedlist.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/logging.h"
)

target_include_directories(utils PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

target_add_test(utils test_id_mapper ${CMAKE_CURRENT_LIST_DIR}/test/test_id_mapper.c)
target_add_test(utils test_linkedlist ${CMAKE_CURRENT_LIST_DIR}/test/test_linkedlist.c)
target_add_test(utils test_linkedlist_iter_action ${CMAKE_CURRENT_LIST_DIR}/test/test_linkedlist_iter_action.c)
