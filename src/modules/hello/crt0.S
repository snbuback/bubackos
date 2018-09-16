.extern module_init
.globl _start, _syscall0, _syscall1, _syscall2, _syscall3, _syscall4, _syscall, _exit

.code64

.text
_start:
    call       module_init
    call       _exit

/**
* Kernel synchronous system call
*/
_syscall0:
    mov    $0, %rsi
_syscall1:
    mov    $0, %rdx
_syscall2:
    mov    $0, %rcx
_syscall3:
    mov    $0, %r8
_syscall4:
    mov    $0, %r9
_syscall:
    // syscall calling convention is different from function calling convention
    // more info on: https://stackoverflow.com/questions/2535989/what-are-the-calling-conventions-for-unix-linux-system-calls-on-i386-and-x86-6
    mov    %rcx, %r10
    syscall
    retq


# exit the current process
_exit:
    movq $1, %rdi
    call _syscall0
    jmp _exit      // this should never happens... but in case...

