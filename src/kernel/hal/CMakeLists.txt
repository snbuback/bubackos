
target_sources(${OS_EXEC_TARGET}
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/multiboot.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/multiboot.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/boot.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/boot.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/crti.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/crtn.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/gdt.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/gdt.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/idt_fill.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/idt.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/idt.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_logging.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_pagging.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_task.S"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/native_task.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/serial.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/x86_64/linker.ld"
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/boot.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/configuration.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/debugger.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/multiboot2_spec.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/native_logging.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/native_task.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/platform.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/hal/tss.h"

    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/gdt.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/idt.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/native_task.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/serial.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/x86_64/tss.h"
)

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

target_add_test(${OS_EXEC_TARGET} test_gdt ${CMAKE_CURRENT_LIST_DIR}/test/test_gdt.c)
target_add_test(${OS_EXEC_TARGET} test_native_pagging ${CMAKE_CURRENT_LIST_DIR}/test/test_native_pagging.c)
