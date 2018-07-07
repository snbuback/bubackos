// x86_64-elf-gcc -g0 -march=nehalem -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c hello.c
// x86_64-elf-readelf -aw ./a.out 
// x86_64-elf-ld --gc-sections -T module.ld hello.o
// x86_64-elf-objdump -dxs ./a.out 

const char stack[] __attribute__ ((section (".interp"))) = "KERNEL_MODULE";

void strcpy(char* dst, char* src) {
    while (*src) {
        *dst = *src;
        ++src; ++dst;
    };
}

/*int x = 4;
char data[452];
*/
void module_init(int arg)
{
    char* msg = "H e l l o   W o r d";
/*    if (data[0] == 1) {
        data[1]++;
        x++;
    }*/
    strcpy((char*) (0xb8000 + 20*80+50), msg);
    if (arg == 3456) {
        strcpy((char*) (0xb8000 + 30*80+50), msg);
    }
}
