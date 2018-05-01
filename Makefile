BASE_DIR=$(CURDIR)
include $(BASE_DIR)/gradle.properties

OS_NAME_LOWER=$(shell echo $(OS_NAME) | tr '[:upper:]' '[:lower:]')
BUILD_DIR=$(BASE_DIR)/build
TARGET=$(OS_ARCH)-elf
SYSROOT=$(BASE_DIR)/sysroot

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CONTAINER=docker run --rm -it -v $(BASE_DIR):$(BASE_DIR) -w $(BASE_DIR) bubackos
else
    CONTAINER=
endif


.PHONY: all build run run-debug shell gdb docker-build
.SUFFIXES:

default: iso

clean:
	@echo cleaning...
	@rm -rf build kernel/build

# build/%.o: js/%.js prepare
# @$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 --rename-section .data=.js $< $@

# gen_load_all_js_module: $(SRC_DIR)/loader/javascript/gen_load_all_js_module.c

# $(SRC_DIR)/loader/javascript/gen_load_all_js_module.c: gen_js_load_all.awk $(js_object_files)
# @find js -name \*.js | awk -f gen_js_load_all.awk > $@

build:
	@$(CONTAINER) ./gradlew build

iso:
	@$(CONTAINER) ./gradlew iso

run:
	@qemu-system-x86_64 -m 128 -cpu Nehalem -cdrom $(BUILD_DIR)/bubackos.iso -no-reboot -no-shutdown -monitor stdio -d cpu_reset,guest_errors,unimp,page

run-debug:
	@qemu-system-x86_64 -m 128 -cpu Nehalem -cdrom $(BUILD_DIR)/bubackos.iso -no-reboot -no-shutdown -monitor stdio -S -s -d cpu_reset,guest_errors,unimp,int,page,in_asm

shell:
	@$(CONTAINER) bash

gdb:
	@$(CONTAINER) gdb -iex 'file build/bootloader/boot/bubackos.elf' -iex 'target remote docker.for.mac.localhost:1234' -iex 'break intel_start'  -iex 'continue'

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
