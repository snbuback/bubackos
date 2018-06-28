
EXE=../../build/bootloader/boot/hello.elf

run:
	x86_64-elf-gcc -g0 -fPIC -march=nehalem -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c hello.c
	x86_64-elf-gcc -x assembler -march=nehalem -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c userspace_init.S
	x86_64-elf-ld -z common-page-size=4096 -z max-page-size=4096 -T module.ld userspace_init.o hello.o -o $(EXE)
	rm userspace_init.o hello.o


readelf:
	x86_64-elf-readelf -aW $(EXE)

objdump:
	x86_64-elf-objdump -wdxs $(EXE)
