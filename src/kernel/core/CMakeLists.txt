
target_sources(${OS_EXEC_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/pmem/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/vmem/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/vmem/region_services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/task/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/scheduler/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/loader/elfparser.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/loader/module_loader.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/logging/services.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/hw_events.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/alloc.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/syscall.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/init.c"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/include/core/pmem/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/vmem/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/vmem/region_services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/task/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/scheduler/services.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/loader/elf.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/loader/module_loader.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/alloc.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/types.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/core/init.h"
)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

target_add_test(${OS_EXEC_TARGET} test_task_services ${CMAKE_CURRENT_LIST_DIR}/test/test_task_services.c)
target_add_test(${OS_EXEC_TARGET} test_vmem_services ${CMAKE_CURRENT_LIST_DIR}/test/test_vmem_services.c)
target_add_test(${OS_EXEC_TARGET} test_loader_elfparser ${CMAKE_CURRENT_LIST_DIR}/test/test_loader_elfparser.c)
target_add_test(${OS_EXEC_TARGET} test_alloc ${CMAKE_CURRENT_LIST_DIR}/test/test_alloc.c)
