BASE_DIR=$(abspath $(CURDIR)/..)
include $(BASE_DIR)/bubackos.conf
include $(BASE_DIR)/Makefile.common

build_dir = $(BASE_DIR)/build
loader.iso := $(build_dir)/loader.iso
loader_build_dir = $(build_dir)/loader
grub-mkrescue = $(CONTAINER) grub-mkrescue

.PHONY: all clean

all: $(loader.iso)

clean:
	@rm -rf $(loader.iso) $(loader_build_dir)

