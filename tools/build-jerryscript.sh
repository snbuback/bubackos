#!/bin/bash
# abort on any error
set -e
set -x

# requires: BUILD_DIR, SYSROOT, CROSS_TRIPLE

if [ ! "$#" -eq "1" ]; then
    echo "Missing jerryscript version number"
    exit 1
fi

JERRYSCRIPT_VERSION=$1

mkdir -p ${BUILD_DIR}

echo "Building Jerryscript ${JERRYSCRIPT_VERSION}"
curl -Ls https://github.com/jerryscript-project/jerryscript/archive/${JERRYSCRIPT_VERSION}.tar.gz | tar -zx -C ${BUILD_DIR}
cd ${BUILD_DIR}/jerryscript-${JERRYSCRIPT_VERSION}

# Fake crt0 just to the compiler check
x86_64-elf-gcc -c /tools/crt0.S -o /usr/local/lib/gcc/x86_64-elf/6.3.0/crt0.o

python3 tools/build.py -v --lto=off --jerry-libc=off --jerry-cmdline=off --jerry-libm=off --toolchain=/tools/toolchain_bubackos.cmake
mkdir -p /usr/local/x86_64-elf/include/jerryscript && cp -v build/lib/*.a /usr/local/x86_64-elf/lib/ && cp -rv jerry-core/include/* jerry-ext/include/* /usr/local/x86_64-elf/include/jerryscript

# fix the include files
for i in `find /usr/local/x86_64-elf/include/jerryscript -type f -name \*.h`; do
    sed -i -e 's/^#include "jerryscript.h"/#include <jerryscript\/jerryscript.h>/' $i
done

