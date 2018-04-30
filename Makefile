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
js_source_files := $(shell find $(JS_DIR) -name *.js)
js_object_files := $(patsubst $(JS_DIR)/%.js, build/%.o, $(js_source_files))
assembly_source_files := $(wildcard $(SRC_DIR)/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst $(SRC_DIR)/%.asm, build/%.o, $(assembly_source_files))
includes_dir := $(shell find $(SRC) -name include -print | sed -e 's/^/-I/' | tr '\n' ' ')
# -I$(SRC_DIR)/include -I$(SRC_DIR)/arch/x86_64
LOADER_BUILD_DIR = $(BUILD_DIR)/loader
LOADER_SRC_DIR = $(BASE_DIR)/loader
GRUB-MKRESCUE = $(CONTAINER) grub-mkrescue

.PHONY: all clean run iso prepare loader test libc jslib
.SUFFIXES:

all: clean gen_load_all_js_module loader

clean:
	@echo cleaning...
	@rm -rf build

prepare:
	@find $(SRC_DIR) -type d | sed -e 's/src/build/' | xargs mkdir -p
	@mkdir -p `dirname $(kernel)`

$(kernel): $(assembly_object_files) $(c_object_files) $(s_object_files) $(linker_script) $(cpp_object_files) $(js_object_files) prepare
	@$(LD) -n -T $(linker_script) -nostdlib -o $(kernel) \
		$(assembly_object_files) $(c_object_files) $(s_object_files) $(cpp_object_files) $(js_object_files) \
		-L/usr/local/x86_64-elf/lib -ljerry-core -ljerry-ext -ljerry-port-default-minimal -lg -lm

build/%.o: src/%.asm prepare
	@$(NASM) -g -felf64 $< -o $@

build/%.o: src/%.cpp prepare
	@$(CPP) -m64 -g -march=nehalem -std=c++99 -ffreestanding -Wall -Wextra -fno-exceptions -mno-red-zone -fno-rtti $(includes_dir) -c $< -o $@

build/%.o: src/%.c prepare
	@$(CC) -m64 -g -march=nehalem -std=gnu99 -ffreestanding -Wall -mno-red-zone -Wextra $(includes_dir) -c $< -o $@

build/%.o: src/%.S prepare
	@$(CC) -m64 -g -Wall $(includes_dir) -c $< -o $@

build/%.o: js/%.js prepare
	@$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=.js $< $@

gen_load_all_js_module: $(SRC_DIR)/loader/javascript/gen_load_all_js_module.c

$(SRC_DIR)/loader/javascript/gen_load_all_js_module.c: gen_js_load_all.awk $(js_object_files)
	@find js -name \*.js | awk -f gen_js_load_all.awk > $@

loader: loader/boot/grub/grub.cfg
	mkdir -p $(LOADER_BUILD_DIR)
	cp -a $(LOADER_SRC_DIR)/boot $(LOADER_BUILD_DIR)
	cp -a $(kernel) $(LOADER_BUILD_DIR)/boot
	$(GRUB-MKRESCUE) -o $(BUILD_DIR)/loader.iso $(LOADER_BUILD_DIR)

run:
	@qemu-system-x86_64 -m 128 -cpu Nehalem -cdrom $(BUILD_DIR)/loader.iso -no-reboot -no-shutdown -monitor stdio -d cpu_reset,guest_errors,unimp,page

run-debug:
	@qemu-system-x86_64 -m 128 -cpu Nehalem -cdrom $(BUILD_DIR)/loader.iso -no-reboot -no-shutdown -monitor stdio -S -s -d cpu_reset,guest_errors,unimp,int,page,in_asm

shell:
	@$(CONTAINER) bash

gdb:
	@$(CONTAINER) gdb -iex 'file build/loader/boot/kernel.bin' -iex 'target remote docker.for.mac.localhost:1234' -iex 'break intel_start'  -iex 'continue'

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
	--build-arg JERRYSCRIPT_VERSION=$(JERRYSCRIPT_VERSION) \
	-t bubackos:latest .
	# build sysroot (just to auto-complete in visual studio)
	@$(CONTAINER) bash -c 'rm -rf $(SYSROOT) && \
		mkdir -p $(SYSROOT)/gcc && cp -a /usr/local/lib/gcc/x86_64-elf/6.3.0/include $(SYSROOT)/gcc && \
		mkdir -p $(SYSROOT)/platform && cp -a /usr/local/x86_64-elf/include $(SYSROOT)/platform/'
