.extern module_init
.globl _start

.code64

.text
_start:
    call       module_init
    call       _exit

// rdi should have the interruption number
_syscall:
    int $50

# exit the current process
_exit:
    movq $1, %rdi
    call _syscall
    jmp _exit      // this should never happens... but in case...

