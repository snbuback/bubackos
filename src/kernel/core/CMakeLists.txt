
target_sources(${OS_EXEC_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/elfparser.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/init.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/memory_management.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/memory.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/page_allocator.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/syscall.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/task_management.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include/core/elf.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/init.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/memory_management.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/memory.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/page_allocator.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/syscall.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/task_management.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/types.h"
)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

target_add_test(${OS_EXEC_TARGET} test_elfparser ${CMAKE_CURRENT_LIST_DIR}/test/test_elfparser.c)
target_add_test(${OS_EXEC_TARGET} test_memory_management ${CMAKE_CURRENT_LIST_DIR}/test/test_memory_management.c)
target_add_test(${OS_EXEC_TARGET} test_task_management ${CMAKE_CURRENT_LIST_DIR}/test/test_task_management.c)
