BASE_DIR=$(CURDIR)
include $(BASE_DIR)/gradle.properties

OS_NAME_LOWER=$(shell echo $(OS_NAME) | tr '[:upper:]' '[:lower:]')
BUILD_DIR=$(BASE_DIR)/build
TARGET=$(OS_ARCH)-elf
SYSROOT=$(BASE_DIR)/sysroot
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CONTAINER=docker run --rm -it --privileged -v $(BASE_DIR):$(BASE_DIR) -w $(BASE_DIR) bubackos
else
    CONTAINER=
endif
QEMU_ARGS=-m 128 -cpu Nehalem -boot order=d -cdrom $(BUILD_DIR)/bubackos.iso -no-reboot -no-shutdown -usb -device usb-tablet -show-cursor -d guest_errors,unimp,page


.PHONY: all build run run-debug shell gdb docker-build
.SUFFIXES:

default: iso

clean:
	@echo cleaning...
	@rm -rf build src/build

# build/%.o: js/%.js prepare
# @$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=.js $< $@

# gen_load_all_js_module: $(SRC_DIR)/loader/javascript/gen_load_all_js_module.c

# $(SRC_DIR)/loader/javascript/gen_load_all_js_module.c: gen_js_load_all.awk $(js_object_files)
# @find js -name \*.js | awk -f gen_js_load_all.awk > $@

prepare-build: clean
	@$(CONTAINER) bash -c 'cd src ; cmake -DCMAKE_TOOLCHAIN_FILE=/Users/snbuback/Projects/bubackos/intel-x86_64.cmake -H. -Bbuild -G "Unix Makefiles"'


build:
	@$(CONTAINER) bash -c '(cd src && cmake --build build)'

iso: build
	@$(CONTAINER) bash -c 'rm -rf build ; mkdir -p build && cp -Rv bootloader build && cp -v src/build/kernel.elf build/bootloader/boot/ && \
	grub-mkrescue -o build/bubackos.iso build/bootloader'

test:
	@$(CONTAINER) ./gradlew kernel:run_tests

run:
	@qemu-system-x86_64 $(QEMU_ARGS) -monitor stdio

run-debug:
	@qemu-system-x86_64 $(QEMU_ARGS) -monitor stdio -S -s

gdb:
	@$(CONTAINER) gdb -iex 'file $(BUILD_DIR)/bootloader/boot/bubackos.elf' -iex 'target remote docker.for.mac.localhost:1234' -iex 'break intel_start'  -iex 'continue'

debug:
	@$(CONTAINER) tmux \
		new-session -d \
			qemu-system-x86_64 $(QEMU_ARGS) -S -s -display curses -monitor /dev/pts/3 -d guest_errors,unimp,page,in_asm,int -D /dev/pts/3 \; \
		split-window -d \
			./tools/gdb-startup.sh localhost:1234 \; \
		split-window -h -d \
			bash -c 'sleep 1; tail --pid=`pgrep qemu-system` -f /dev/null' \; \
		select-pane -D \; \
		resize-pane -t 0 -x 20 -y 25 \; \
		set mouse on \; \
		attach


dump-symbols:
	@$(CONTAINER) objdump -t $(BUILD_DIR)/bootloader/boot/bubackos.elf  | sort -n

dump-asm:
	@$(CONTAINER) objdump -xd $(BUILD_DIR)/bootloader/boot/bubackos.elf 


shell:
	@$(CONTAINER) bash

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
