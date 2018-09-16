#include <stdint.h>
#include <string.h>
// x86_64-elf-gcc -g0 -march=nehalem -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c hello.c
// x86_64-elf-readelf -aw ./a.out 
// x86_64-elf-ld --gc-sections -T module.ld hello.o
// x86_64-elf-objdump -dxs ./a.out 

const char stack[] __attribute__ ((section (".interp"))) = "KERNEL_MODULE";

long syscall(long arg1, long arg2, long arg3, long arg4, long arg5)
{
    //asm volatile ("movq %0, %%cr3" : : "r" (pt->entries));
    long result;
    asm ("\
        mov %1, %%rdi; \
        mov %2, %%rsi; \
        mov %3, %%rdx; \
        mov %4, %%rcx; \
        mov %5, %%r8; \
        syscall; \
        movq %%rax, %0"
    : "=r"(result)
    : "m"(arg1), "m"(arg2), "m"(arg3), "m"(arg4), "m"(arg5));
    return result;
}

long syscall3(long arg1, long arg2, long arg3)
{
    return syscall(arg1, arg2, arg3, 0, 0);
}

typedef struct {
    uint64_t num_arguments;
    char** argument_list; // char* arguments[]
} task_userdata_t;

static void print(int line, int col, const char* msg)
{
    char c;
    char* base = (char*) ((uintptr_t) 0xb8000 + (line*80+col)*2);
    while ((c = *msg)) {
        *base = c;
        *(base+1) = ' ';
        msg++;
        base += 2;
    }
}

task_userdata_t* userdata;
void module_init(uintptr_t arg)
{
    userdata = (task_userdata_t*) arg;

    print(0, strlen("1234567890"), "Hello  Word");

    syscall(0x1000, 0x2000, 0x3000, 0x4000, 0x5000);
    
    syscall(0x1001, 0x2001, 0x3001, 0x4001, 0x5001);

    syscall3(2/*logging*/, 2 /*INFO*/, (long) "logging from application");

    if (!userdata) {
        print(1, 0, "NO Arguments!!!");
    } else {
        print(1, 0, "Arguments:");
        for (uint64_t i=0; i<userdata->num_arguments; ++i) {
            print(i+2, 0, userdata->argument_list[i]);
        }
    }
}
