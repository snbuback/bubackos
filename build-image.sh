#!/bin/bash
set -xue

SYSROOT=$PWD/sysroot
source dependencies.properties

docker image build --pull --squash --build-arg SYSROOT=$SYSROOT \
--build-arg CROSS_TRIPLE=$TARGET \
--build-arg BINUTILS_VERSION=$BINUTILS_VERSION \
--build-arg GCC_VERSION=$GCC_VERSION \
--build-arg NEWLIB_VERSION=$NEWLIB_VERSION \
-t bubackos:latest .

# build sysroot (just to auto-complete in visual studio)
rm -rf $SYSROOT
mkdir -p $SYSROOT/gcc
docker run --rm -it -v "$SYSROOT:$SYSROOT" -w "$SYSROOT"  bubackos:latest cp -vr /usr/local/lib/gcc/$TARGET/$GCC_VERSION/include $SYSROOT/gcc/

