BASE_DIR=$(CURDIR)
include $(BASE_DIR)/dependencies.properties

TARGET=$(OS_ARCH)-elf
SYSROOT=$(BASE_DIR)/sysroot
BUILD_DIR=$(BASE_DIR)/build
MODULES_DIR=$(BUILD_DIR)/bootloader/boot
KERNEL_IMAGE=$(BUILD_DIR)/bootloader/boot/kernel.elf

.PHONY: all build run run-debug shell gdb docker-build
.SUFFIXES:

default: iso

clean: module-clean
	@echo cleaning...
	@rm -rf build src/build

prepare-build: clean module-prepare-build
	@cd src && cmake -DCMAKE_TOOLCHAIN_FILE=$(BASE_DIR)/src/intel-x86_64.cmake -H. -Bbuild -G "Ninja"

build:
	@cd $(BASE_DIR)/src && cmake --build build

module-clean:
	@find src/modules -name build -type d | xargs rm -rf

module-prepare-build: module-clean
	@cd src/modules ; for module in `cat modules.list`; do \
		echo "Building $$module"; cd $$module && cmake -DCMAKE_TOOLCHAIN_FILE=$(BASE_DIR)/src/intel-x86_64.cmake -H. -Bbuild -G "Ninja" ; \
	done

module-build:
	@cd src/modules ; for module in `cat modules.list`; do \
		echo "Building $$module"; cd $$module && cmake --build build; \
	done

lint:
	@cd $(BASE_DIR)/src && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -H. build && \
		cd $(BASE_DIR) && oclint-json-compilation-database -p $(BASE_DIR)/src/build

test: build lint
	@cd $(BASE_DIR)/src/build && ctest -VV

iso: build module-build
	@rm -rf $(BASE_DIR)/build
	@mkdir -p $(BASE_DIR)/build
	@cp -Rv $(BASE_DIR)/bootloader $(BASE_DIR)/build
	@cp -v $(BASE_DIR)/src/build/kernel.elf $(KERNEL_IMAGE)
	@for module in `cat src/modules/modules.list`; do cp -v $(BASE_DIR)/src/modules/$$module/build/$$module.mod $(MODULES_DIR); done
	@grub-mkrescue -o $(BASE_DIR)/build/myos.iso $(BASE_DIR)/build/bootloader

run:
	@./tools/start-emulator.sh $(BUILD_DIR)/myos.iso

run-debug:
	@./tools/start-emulator.sh $(BUILD_DIR)/myos.iso $(KERNEL_IMAGE)

gdb:
	@gdb -iex 'file $(KERNEL_IMAGE)' -iex 'target remote docker.for.mac.localhost:1234' -iex 'break intel_start'  -iex 'continue'

dump-symbols:
	@objdump -t $(KERNEL_IMAGE) | sort -n

dump-asm:
	@bash -c 'objdump -d --section=.text build/bootloader/boot/*.{elf,mod}'

docker-build:
	docker build --build-arg SYSROOT=$(SYSROOT) \
	--build-arg CROSS_TRIPLE=$(TARGET) \
	--build-arg BINUTILS_VERSION=$(BINUTILS_VERSION) \
	--build-arg GCC_VERSION=$(GCC_VERSION) \
	--build-arg NEWLIB_VERSION=$(NEWLIB_VERSION) \
	-t bubackos:latest .
	# build sysroot (just to auto-complete in visual studio)
	@bash -c 'rm -rf $(SYSROOT) && \
		mkdir -p $(SYSROOT)/gcc && cp -a /usr/local/lib/gcc/x86_64-elf/$(GCC_VERSION)/include $(SYSROOT)/gcc'
