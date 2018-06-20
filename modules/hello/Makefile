
run:
	@x86_64-elf-gcc -g0 -march=nehalem -std=gnu99 -ffreestanding -Wall -Werror -Wextra -DNDEBUG -mno-red-zone -c hello.c && \
	x86_64-elf-ld --dynamic-linker=/module/sdsd.so -z common-page-size=4096 -z max-page-size=4096 -T module.ld hello.o -o ../../build/bootloader/boot
	@# x86_64-elf-readelf -aw ./hello.elf
	@# x86_64-elf-objdump -dxs ./hello.elf
