
target_sources(${OS_EXEC_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/task/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/scheduler/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/vmem/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/vmem/region_services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/loader/elfparser.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/loader/module_loader.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/init.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/memory.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/page_allocator.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/syscall.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include/core/task/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/scheduler/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/vmem/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/vmem/region_services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/loader/elf.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/loader/module_loader.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/init.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/memory.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/page_allocator.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/syscall.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/types.h"
)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

target_add_test(${OS_EXEC_TARGET} test_task_services ${CMAKE_CURRENT_LIST_DIR}/test/test_task_services.c)
target_add_test(${OS_EXEC_TARGET} test_vmem ${CMAKE_CURRENT_LIST_DIR}/test/test_vmem.c)
target_add_test(${OS_EXEC_TARGET} test_elfparser ${CMAKE_CURRENT_LIST_DIR}/test/test_elfparser.c)
target_add_test(${OS_EXEC_TARGET} test_memory ${CMAKE_CURRENT_LIST_DIR}/test/test_memory.c)
