#include <stdint.h>
// x86_64-elf-gcc -g0 -march=nehalem -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c hello.c
// x86_64-elf-readelf -aw ./a.out 
// x86_64-elf-ld --gc-sections -T module.ld hello.o
// x86_64-elf-objdump -dxs ./a.out 

const char stack[] __attribute__ ((section (".interp"))) = "KERNEL_MODULE";

typedef struct {
    uint64_t num_arguments;
    char** argument_list; // char* arguments[]
} task_userdata_t;

void strcpy(char* dst, const char* src) {
    while (*src) {
        *dst = *src;
        ++src; ++dst;
    };
}

static void print(int line, int col, const char* msg)
{
    char c;
    char* base = (char*) ((uintptr_t) 0xb8000 + (line*80+col)*2);
    asm ("xchgw %bx, %bx");
    while ((c = *msg)) {
        *base = c;
        *(base+1) = ' ';
        msg++;
        base += 2;
    }
}

task_userdata_t* userdata;
void module_init()
{
    // userdata = (task_userdata_t*) arg;

    print(0, 0, "Hello  Word");
    print(1, 15, "Argumentos:");

    // for (uint64_t i=0; i<userdata->num_arguments; ++i) {
    //     print(i+2, 0, userdata->argument_list[i]);
    // }
}
