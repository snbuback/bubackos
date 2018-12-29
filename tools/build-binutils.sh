#!/bin/bash
# abort on any error
set -e
set -x

# requires: BUILD_DIR, SYSROOT, CROSS_TRIPLE

if [ ! "$#" -eq "1" ]; then
    echo "Missing version number"
    exit 1
fi

BINUTILS_VERSION=$1

TOOLS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
mkdir -p ${BUILD_DIR}
echo "Building binutils ${BINUTILS_VERSION}"
curl -s ftp://ftp.gnu.org/gnu/binutils//binutils-${BINUTILS_VERSION}.tar.gz | tar -zx -C ${BUILD_DIR}
cd ${BUILD_DIR}/binutils-${BINUTILS_VERSION}
./configure --target=${CROSS_TRIPLE} --disable-nls --disable-werror
make
make install
