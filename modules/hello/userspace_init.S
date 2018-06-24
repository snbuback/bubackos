
#define SYSTEM_STACKSIZE        4096

.extern module_init
.globl _start

.code64

.text
_start:

    movq       $0x3456, %rdi   # just a test
    # setup the user stack
    movq       $_stack_area, %rbp
    addq       $4000, %rbp
    movq       %rbp, %rsp

    call       module_init
    call       _exit

# exit the current process
_exit:
    movq $1, %rdi
    syscall
    jmp _exit      // this should never happens... but in case...

.bss
.lcomm _stack_area, 4096

