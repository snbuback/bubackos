#!/bin/bash
# abort on any error
set -e
set -x

# requires: BUILD_DIR, SYSROOT, CROSS_TRIPLE
# patch file generated after git add . with: git diff --cached --binary > /tmp/mypatch.patch

if [ ! "$#" -eq "2" ]; then
    echo "Missing gcc and newlib version number"
    exit 1
fi

GCC_VERSION=$1
NEWLIB_VERSION=$2

TOOLS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
mkdir -p ${BUILD_DIR}

echo "Building Newlib ${NEWLIB_VERSION}"
curl -s ftp://sourceware.org/pub/newlib/newlib-${NEWLIB_VERSION}.tar.gz | tar -zx -C ${BUILD_DIR}
cd ${BUILD_DIR}/newlib-${NEWLIB_VERSION}
# patch -p1 < ${BUILD_DIR}/patches/newlib.patch
./configure --target=${CROSS_TRIPLE} --disable-nls --disable-werror
make
make install

# rebuild gcc with newlib
echo "Rebuild GCC with Newlib"
cd ${BUILD_DIR}/build-gcc
${BUILD_DIR}/gcc-${GCC_VERSION}/configure --target=${CROSS_TRIPLE} --disable-nls --enable-languages=c --with-newlib --disable-shared --disable-libssp
../gcc-${GCC_VERSION}/configure --target=${CROSS_TRIPLE} --disable-nls --enable-languages=c --with-newlib --disable-shared --disable-libssp
make all
make install
