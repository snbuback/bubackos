
target_sources(${OS_EXEC_TARGET}
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/common/multiboot2.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/lib/gdt.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/lib/idt_fill.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/lib/idt.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/lib/idt.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/multiboot2.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/boot.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/boot.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_pagging.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_task.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_task.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_logging.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/serial.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/crti.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/crtn.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/linker.ld"
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include/common/multiboot2.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/native_logging.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/native_task.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/gdt.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/idt.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/native_task.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/serial.h"
)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

target_add_test(${OS_EXEC_TARGET} test_gdt ${CMAKE_CURRENT_LIST_DIR}/test/test_gdt.c)
target_add_test(${OS_EXEC_TARGET} test_native_pagging ${CMAKE_CURRENT_LIST_DIR}/test/test_native_pagging.c)
