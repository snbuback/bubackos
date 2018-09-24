# BubackOS

This OS is my personal training exercise about how to write an operational system. Also, it is an excellent opportunity to exercise system design (I recommend everybody trying the same).

## Design guidelines
* Performance it is not a concern, but a good system design is.
* Maintainability and Reliability are important
* The kernel should be minimal as possible (micro-kernel or nano-kernel). In case of failure of part of a module, the rest of the system should keep working if they don't depend on the module.
* My goal is just 64 bits machines. Makes things easier and also stay up-to-date with the latest changes in CPU architecture.
* Should be portable to ARM64.
* Multi-task but not multi-user
* Only text interface

## Weird things I would like to accomplish
* The concept of a process should not be part of the kernel. I would like to try to write loaders, and these loaders should create their concept of Process. Probably I would like to have a LoaderManager (I will try don't add this to the kernel)
* Modules should be similar to a user process. They should ask the kernel about particular memory address they want to have access.
* IPC should be nice :-) and very well design.
* Asynchronous syscalls. "All" call should be a message in the IPC. The task can wait for the answer if they want or should write messages in the IPC and mark as "I don't want the answer".

## Directory structure

* src
 * arch: assembly code for architecture depend parts
 * include: include files (kernel and libc to the kernel)
 * kernel: c files
 * libc: libc tiny implementation
tools: tools like create iso image
build-tools: gcc and binutils to build the kernel

## Debugging

To start a qemu in debug mode

    make debug

To start the gdb inside the docker

    make gdb

### Breakpoint in bochs

Intel: ```xchg bx, bx```
GAS: ```xchgw %bx, %bx```

Requires parameter ```magic_break: enabled=1```


## Useful qemu commands
* info mem - show the active virtual memory mappings
* info tlb - show virtual to physical memory mappings
* info registers

## References
* https://wiki.osdev.org/Main_Page
* https://github.com/torvalds/linux

## TODO
* Create a new kernel_mocks, mocking the entire kernel (maybe including mocking from all services)
* When create new region of memory, verify if there is no overlap
* Creates the DEBUGGER keyboard with pre-defined breakpoints in debug mode.
