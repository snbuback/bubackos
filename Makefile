BASE_DIR=$(CURDIR)
include $(BASE_DIR)/dependencies.properties

OS_NAME_LOWER=$(shell echo $(OS_NAME) | tr '[:upper:]' '[:lower:]')
BUILD_DIR=$(BASE_DIR)/build
TARGET=$(OS_ARCH)-elf
SYSROOT=$(BASE_DIR)/sysroot
UNAME_S := $(shell uname -s)
MODULES_DIR=$(BUILD_DIR)/bootloader/boot
KERNEL_IMAGE=$(BUILD_DIR)/bootloader/boot/kernel.elf
ifeq ($(UNAME_S),Darwin)
    CONTAINER=docker run --rm -it --privileged -v $(BASE_DIR):$(BASE_DIR) -w $(BASE_DIR) bubackos
else
    CONTAINER=
endif
QEMU_ARGS=-m 128 -cpu Nehalem -boot order=d -cdrom $(BUILD_DIR)/bubackos.iso -no-reboot \
	-no-shutdown -usb -device usb-tablet -icount auto,sleep=on \
	-serial file:/dev/tty \
	-show-cursor -d guest_errors,unimp,page,cpu_reset


.PHONY: all build run run-debug shell gdb docker-build
.SUFFIXES:

default: iso

clean:
	@echo cleaning...
	@rm -rf build src/build

prepare-build: clean
	@$(CONTAINER) bash -c 'cd src ; cmake -DCMAKE_TOOLCHAIN_FILE=$(BASE_DIR)/src/intel-x86_64.cmake -H. -Bbuild -G "Unix Makefiles"'

build:
	@$(CONTAINER) bash -c '(cd $(BASE_DIR)/src && cmake --build build)'

test: build
	@$(CONTAINER) bash -c '(cd $(BASE_DIR)/src/build && ctest -VV)'

module-clean:
	@find src/modules -name build -type d | xargs rm -rf

module-prepare-build: module-clean
	@$(CONTAINER) bash -c 'cd src/modules ; for module in `cat modules.list`; do \
		echo "Building $$module"; cd $$module && cmake -DCMAKE_TOOLCHAIN_FILE=$(BASE_DIR)/src/intel-x86_64.cmake -H. -Bbuild -G "Unix Makefiles" ; \
	done'

module-build:
	@$(CONTAINER) bash -c 'cd src/modules ; for module in `cat modules.list`; do \
		echo "Building $$module"; cd $$module && cmake --build build; \
	done'

iso: build module-build
	@$(CONTAINER) bash -c ' \
	rm -rf $(BASE_DIR)/build ; \
	mkdir -p $(BASE_DIR)/build && \
	cp -Rv $(BASE_DIR)/bootloader $(BASE_DIR)/build && \
	cp -v $(BASE_DIR)/src/build/kernel.elf $(KERNEL_IMAGE) && \
	for module in `cat src/modules/modules.list`; do cp -v $(BASE_DIR)/src/modules/$$module/build/$$module.mod $(MODULES_DIR); done ; \
	grub-mkrescue -o $(BASE_DIR)/build/bubackos.iso $(BASE_DIR)/build/bootloader'

run:
	@qemu-system-x86_64 $(QEMU_ARGS) -monitor stdio

start-debug:
	tmux new-session -d qemu-system-x86_64 $(QEMU_ARGS) -S -s

stop-debug:
	killall qemu-system-x86_64

gdb:
	@$(CONTAINER) gdb -iex 'file $(KERNEL_IMAGE)' -iex 'target remote docker.for.mac.localhost:1234' -iex 'break intel_start'  -iex 'continue'

debug:
	@$(CONTAINER) tmux \
		new-session -d \
			qemu-system-x86_64 $(QEMU_ARGS) -S -s -display none -monitor /dev/pts/3 -d guest_errors,unimp,page,in_asm,int -D /dev/pts/3 \; \
		split-window -d \
			./tools/gdb-startup.sh $(KERNEL_IMAGE) localhost:1234 \; \
		split-window -h -d \
			bash -c 'sleep 1; tail --pid=`pgrep qemu-system` -f /dev/null' \; \
		select-pane -D \; \
		set mouse on \; \
		attach

dump-symbols:
	@$(CONTAINER) objdump -t $(KERNEL_IMAGE)  | sort -n

dump-asm:
	@$(CONTAINER) objdump -xd $(KERNEL_IMAGE)

shell:
	@$(CONTAINER) bash

docker-build:
	docker build --build-arg SYSROOT=$(SYSROOT) \
	--build-arg CROSS_TRIPLE=$(TARGET) \
	--build-arg BINUTILS_VERSION=$(BINUTILS_VERSION) \
	--build-arg GCC_VERSION=$(GCC_VERSION) \
	--build-arg NEWLIB_VERSION=$(NEWLIB_VERSION) \
	-t bubackos:latest .
	# build sysroot (just to auto-complete in visual studio)
	@$(CONTAINER) bash -c 'rm -rf $(SYSROOT) && \
		mkdir -p $(SYSROOT)/gcc && cp -a /usr/local/lib/gcc/x86_64-elf/$(GCC_VERSION)/include $(SYSROOT)/gcc'
