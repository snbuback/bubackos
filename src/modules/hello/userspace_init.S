
#define SYSTEM_STACKSIZE        4096

.extern module_init
.globl _start

.code64

.text
_start:

    xchgw %bx, %bx
    # setup the user stack
    movq       $_stack_area, %rbp
    addq       $4096, %rbp
    movq       %rbp, %rsp

    call       module_init
    call       _exit

# exit the current process
_exit:
    movq $1, %rdi
    int $50
    jmp _exit      // this should never happens... but in case...

.bss
.lcomm _stack_area, 4096

