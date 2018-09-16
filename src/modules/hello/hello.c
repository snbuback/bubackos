#include <stdint.h>
#include <string.h>
// x86_64-elf-gcc -g0 -march=nehalem -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c hello.c
// x86_64-elf-readelf -aw ./a.out 
// x86_64-elf-ld --gc-sections -T module.ld hello.o
// x86_64-elf-objdump -dxs ./a.out 

const char stack[] __attribute__ ((section (".interp"))) = "KERNEL_MODULE";

extern long _syscall0(long syscall_number);
extern long _syscall1(long syscall_number, long arg1);
extern long _syscall2(long syscall_number, long arg1, long arg2);
extern long _syscall3(long syscall_number, long arg1, long arg2, long arg3);
extern long _syscall4(long syscall_number, long arg1, long arg2, long arg3, long arg4);
extern long _syscall(long syscall_number, long arg1, long arg2, long arg3, long arg4, long arg5);

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

    _syscall(0x1005, 0x2005, 0x3005, 0x4005, 0x5005, 0x6005);

    _syscall4(0x1004, 0x2004, 0x3004, 0x4004, 0x5004);

    _syscall3(0x1003, 0x2003, 0x3003, 0x4003);

    _syscall2(0x1002, 0x2002, 0x3002);

    _syscall1(0x1001, 0x2001);

    _syscall0(0x1000);

    _syscall2(2/*logging*/, 2 /*INFO*/, (long) "logging from application");

    if (!userdata) {
        print(1, 0, "NO Arguments!!!");
    } else {
        print(1, 0, "Arguments:");
        for (uint64_t i=0; i<userdata->num_arguments; ++i) {
            print(i+2, 0, userdata->argument_list[i]);
        }
    }
}
