include bubackos.conf
include Makefile.common


BASE_DIR=$(CURDIR)
SRC_DIR=$(CURDIR)/src
arch ?= x86_64
kernel := dist/kernel.bin
linker_script := $(SRC_DIR)/arch/$(arch)/linker.ld
VPATH = %.asm src
# cpp_source_files := $(shell find $(SRC_DIR) -name *.cpp)
# cpp_object_files := $(patsubst $(SRC_DIR)/%.cpp, build/%.o, $(cpp_source_files))
c_source_files := $(shell find $(SRC_DIR) -name *.c)
c_object_files := $(patsubst $(SRC_DIR)/%.c, build/%.o, $(c_source_files))
s_source_files := $(shell find $(SRC_DIR) -name *.S)
s_object_files := $(patsubst $(SRC_DIR)/%.S, build/%.o, $(s_source_files))
assembly_source_files := $(wildcard $(SRC_DIR)/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst $(SRC_DIR)/%.asm, build/%.o, $(assembly_source_files))
includes_dir := -I$(SRC_DIR)/include -I$(SRC_DIR)/arch/x86_64 -I$(SRC_DIR)/libc/include


.PHONY: all clean run iso prepare loader test
.SUFFIXES:

all: $(kernel) loader

clean:
	@rm -rf build dist

prepare:
	@find $(SRC_DIR) -type d | sed -e 's/src/build/' | xargs mkdir -p
	@mkdir -p `dirname $(kernel)`

$(kernel): $(assembly_object_files) $(c_object_files) $(s_object_files) $(linker_script) prepare
	$(LD) -n -T $(linker_script) -nostdlib -o $(kernel) $(assembly_object_files) $(c_object_files) $(s_object_files)

build/%.o: src/%.asm prepare
	$(NASM) -g -felf64 $< -o $@

# build/%.o: src/%.cpp prepare
# 	$(CPP) -m64 -g -std=c++11 -ffreestanding -Wall -Wextra -fno-exceptions -mno-red-zone -fno-rtti $(includes_dir) -c $< -o $@
#
build/%.o: src/%.c prepare
	$(CC) -m64 -g -std=gnu11 -ffreestanding -Wall -mno-red-zone -Wextra $(includes_dir) -c $< -o $@

build/%.o: src/%.S prepare
	$(CC) -m64 -g -Wall $(includes_dir) -c $< -o $@

loader:
	@$(MAKE) -C tools/loader all

run:
	@qemu-system-x86_64 -boot order=d -cdrom dist/loader.iso -hda fat:./dist -no-reboot -no-shutdown -monitor stdio

run-debug:
	@qemu-system-x86_64 -boot order=d -cdrom dist/loader.iso -hda fat:./dist -no-reboot -no-shutdown -d cpu_reset,guest_errors,unimp,in_asm,int,page

test:
	@test/run.py

test-debug:
	@test/run.py -debug
