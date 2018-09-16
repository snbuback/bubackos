.extern module_init
.globl _start

.code64

.text
_start:
    call       module_init
    call       _exit

# exit the current process
_exit:
    movq $1, %rdi
    call syscall
    jmp _exit      // this should never happens... but in case...

