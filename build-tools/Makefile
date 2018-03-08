# Reference: https://wiki.osdev.org/GCC_Cross-Compiler
include ../Makefile.common
BASE_DIR=$(CURDIR)
BINUTILS_DOWNLOAD=ftp://ftp.gnu.org/gnu/binutils//binutils-$(BINUTILS_VERSION).tar.gz
BINUTILS_SRC=$(BASE_DIR)/binutils-$(BINUTILS_VERSION)
GCC_DOWNLOAD=ftp://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.gz
GCC_SRC=$(BASE_DIR)/gcc-$(GCC_VERSION)
GCC_BUILD_DIR=$(BASE_DIR)/build-gcc

# For MacOSX GCC directory using home brew.
export CC=/usr/local/bin/gcc-7
export CXX=/usr/local/bin/g++-7
export CPP=/usr/local/bin/cpp-7
export LD=$(CC)

.PHONY: all binutils gcc clean

all: binutils gcc

clean:
	rm -rf $(BINUTILS_SRC) $(GCC_SRC) $(PREFIX) $(GCC_BUILD_DIR)

## Binutils
$(BINUTILS_SRC):
	cd $(abspath $(BINUTILS_SRC)/..) && curl -s $(BINUTILS_DOWNLOAD) | tar -zxv

binutils: $(BINUTILS_SRC)
	cd $(BINUTILS_SRC) && find . -name config.cache -delete
	cd $(BINUTILS_SRC) && ./configure --prefix=$(PREFIX) --target=$(TARGET) --enable-interwork --enable-multilib --disable-nls --disable-werror
	$(MAKE) -C $(BINUTILS_SRC)
	$(MAKE) -C $(BINUTILS_SRC) install

## GCC
$(GCC_SRC):
	cd $(abspath $(GCC_SRC)/..) && curl -s $(GCC_DOWNLOAD) | tar -zxv

gcc: $(GCC_SRC)
	rm -rf $(GCC_BUILD_DIR) && mkdir -p $(GCC_BUILD_DIR)
	cd $(GCC_BUILD_DIR) && $(GCC_SRC)/configure --target=$(TARGET) --prefix=$(PREFIX) --enable-interwork --disable-nls \
		--enable-languages=c,c++ --without-headers --enable-multilib \
		--with-gmp=/usr/local --with-mpc=/usr/local --with-mpfr=/usr/local
	$(MAKE) -C $(GCC_BUILD_DIR) all-gcc
	$(MAKE) -C $(GCC_BUILD_DIR) all-target-libgcc
	$(MAKE) -C $(GCC_BUILD_DIR) install-gcc
	$(MAKE) -C $(GCC_BUILD_DIR) install-target-libgcc
