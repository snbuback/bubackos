dist_dir = ../../dist
build_dir = ../../build
loader.iso := $(dist_dir)/loader.iso
loader_build_dir = $(build_dir)/loader
grub-mkrescue = docker run --rm -it -v $(shell cd ../..; pwd):/workdir -w /workdir/tools/loader snbuback/myos grub-mkrescue

.PHONY: all clean

all: $(loader.iso)

clean:
	@rm -rf $(loader.iso) $(loader_build_dir)

$(loader.iso): boot/grub/grub.cfg
	@mkdir -p $(loader_build_dir) $(dist_dir)
	@cp -a boot $(loader_build_dir)
	$(grub-mkrescue) -o $(loader.iso) $(loader_build_dir)
	@rm -rf $(loader_build_dir)
