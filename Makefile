BASE_DIR=$(CURDIR)
include $(BASE_DIR)/bubackos.conf
include $(BASE_DIR)/Makefile.common


arch ?= x86_64
kernel := $(BUILD_DIR)/kernel.bin
linker_script := $(SRC_DIR)/arch/$(arch)/linker.ld
VPATH = %.asm src
cpp_source_files := $(shell find $(SRC_DIR) -name *.cpp)
cpp_object_files := $(patsubst $(SRC_DIR)/%.cpp, build/%.o, $(cpp_source_files))
c_source_files := $(shell find $(SRC_DIR) -name *.c)
c_object_files := $(patsubst $(SRC_DIR)/%.c, build/%.o, $(c_source_files))
s_source_files := $(shell find $(SRC_DIR) -name *.S)
s_object_files := $(patsubst $(SRC_DIR)/%.S, build/%.o, $(s_source_files))
assembly_source_files := $(wildcard $(SRC_DIR)/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst $(SRC_DIR)/%.asm, build/%.o, $(assembly_source_files))
includes_dir := -I$(SRC_DIR)/include -I$(SRC_DIR)/arch/x86_64 -I$(BUILD_DIR)/include
LOADER_BUILD_DIR = $(BUILD_DIR)/loader
LOADER_SRC_DIR = $(BASE_DIR)/loader
GRUB-MKRESCUE = $(CONTAINER) grub-mkrescue

.PHONY: all clean run iso prepare loader test libc jslib
.SUFFIXES:

all: loader

clean:
	@rm -rf build

prepare:
	@find $(SRC_DIR) -type d | sed -e 's/src/build/' | xargs mkdir -p
	@mkdir -p `dirname $(kernel)`

$(kernel): $(assembly_object_files) $(c_object_files) $(s_object_files) $(linker_script) $(cpp_object_files) prepare
	$(LD) -n -T $(linker_script) -nostdlib -o $(kernel) $(assembly_object_files) $(c_object_files) $(s_object_files) $(cpp_object_files) \
		-L$(BUILD_DIR)/lib -lc -lm

build/%.o: src/%.asm prepare
	$(NASM) -g -felf64 $< -o $@

build/%.o: src/%.cpp prepare
	$(CPP) -m64 -g -std=c++99 -ffreestanding -Wall -Wextra -fno-exceptions -mno-red-zone -fno-rtti $(includes_dir) -c $< -o $@

build/%.o: src/%.c prepare
	$(CC) -m64 -g -std=gnu99 -ffreestanding -Wall -mno-red-zone -Wextra $(includes_dir) -c $< -o $@

build/%.o: src/%.S prepare
	$(CC) -m64 -g -Wall $(includes_dir) -c $< -o $@

loader: loader/boot/grub/grub.cfg $(kernel)
	@mkdir -p $(LOADER_BUILD_DIR)
	@cp -a $(LOADER_SRC_DIR)/boot $(LOADER_BUILD_DIR)
	@cp -a $(kernel) $(LOADER_BUILD_DIR)/boot
	$(GRUB-MKRESCUE) -o $(BUILD_DIR)/loader.iso $(LOADER_BUILD_DIR)

run:
	@qemu-system-x86_64 -cdrom $(BUILD_DIR)/loader.iso -no-reboot -no-shutdown -monitor stdio

run-debug:
	@qemu-system-x86_64 -cdrom $(BUILD_DIR)/loader.iso -no-reboot -no-shutdown -d cpu_reset,guest_errors,unimp,in_asm,int,page

shell:
	@$(CONTAINER) bash
test:
	@$(CONTAINER) test/run.py

test-debug:
	@$(CONTAINER) test/run.py -debug

docker-build:
	docker build --build-arg SYSROOT=$(SYSROOT) \
	--build-arg CROSS_TRIPLE=$(TARGET) \
	--build-arg BINUTILS_VERSION=$(BINUTILS_VERSION) \
	--build-arg GCC_VERSION=$(GCC_VERSION) \
	--build-arg NEWLIB_VERSION=$(NEWLIB_VERSION) \
	-t bubackos:latest .

libc:
	@$(CONTAINER) bash -c "cd libc && \
	./configure --with-build-sysroot=$(PREFIX) --target=$(TARGET) --prefix=$(PREFIX) && \
	make && \
	make install && \
	cp -a /opt/cross/x86_64-bubackos-elf $(BUILD_DIR)"

jslib:
	export LDFLAGS="--sysroot=/opt/cross -L/Users/snbuback/Projects/bubackos/build/lib -lgcc -nostdlib"
	python3 tools/build.py -v --lto=off --jerry-libc=off --jerry-cmdline=off --jerry-libm=off --toolchain=../toolchain_bubackos.cmake