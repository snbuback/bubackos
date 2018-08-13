#!/bin/bash
# abort on any error
set -e
set -x

# requires: BUILD_DIR, SYSROOT, CROSS_TRIPLE
# patch file generated after git add . with: git diff --cached --binary > /tmp/mypatch.patch

if [ ! "$#" -eq "1" ]; then
    echo "Missing version number"
    exit 1
fi

GCC_VERSION=$1

TOOLS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
mkdir -p ${BUILD_DIR}
echo "Building gcc ${GCC_VERSION}"
curl -s ftp://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz | tar -zx -C ${BUILD_DIR}
cd ${BUILD_DIR}/gcc-${GCC_VERSION}
# patch -p1 < ${TOOLS_DIR}/patches/gcc.patch
./contrib/download_prerequisites
mkdir -p ${BUILD_DIR}/build-gcc
cd ${BUILD_DIR}/build-gcc
${BUILD_DIR}/gcc-${GCC_VERSION}/configure --target=${CROSS_TRIPLE} --disable-nls --enable-languages=c --without-headers 
make all-gcc
make install-gcc
