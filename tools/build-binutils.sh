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

BINUTILS_VERSION=$1

TOOLS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
mkdir -p ${BUILD_DIR}
echo "Building binutils ${BINUTILS_VERSION}"
curl -s ftp://ftp.gnu.org/gnu/binutils//binutils-${BINUTILS_VERSION}.tar.gz | tar -zx -C ${BUILD_DIR}
cd ${BUILD_DIR}/binutils-${BINUTILS_VERSION}
# patch -p1 < ${TOOLS_DIR}/patches/binutils.patch
(cd ld && AUTOCONF=autoconf2.64 automake-1.11)
./configure --target=${CROSS_TRIPLE} --disable-nls --disable-werror
make
make install
