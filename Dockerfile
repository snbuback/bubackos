FROM debian:stretch
ARG BINUTILS_VERSION=2.28
ARG GCC_VERSION=6.3.0
ARG PREFIX=/opt/cross
ENV CROSS_TRIPLE x86_64-bubackos-elf
ENV PATH ${PATH}:${PREFIX}/bin
ENV LD_LIBRARY_PATH ${PREFIX}/lib:${LD_LIBRARY_PATH}

# Upgrade and requirements
RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y install bash curl wget pkg-config build-essential make automake autogen \
        tar xz-utils bzip2 gzip file rsync sed vim binutils gcc
# BINUTILS
RUN curl -s ftp://ftp.gnu.org/gnu/binutils//binutils-${BINUTILS_VERSION}.tar.gz | tar -zx && \
    cd binutils-${BINUTILS_VERSION} && \
    ./configure --target=${CROSS_TRIPLE} --prefix=${PREFIX} --disable-nls --disable-werror && \
    make && \
    make install && \
    cd / && rm -rf binutils-${BINUTILS_VERSION}
# GCC
RUN curl -s ftp://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz | tar -zx && \
    cd /gcc-${GCC_VERSION} && ./contrib/download_prerequisites && \
    mkdir -p /build-gcc && \
    cd /build-gcc && \
    /gcc-${GCC_VERSION}/configure --target=${CROSS_TRIPLE} --prefix=${PREFIX} --disable-nls --enable-languages=c,c++ --without-headers && \
    make all-gcc && \
    make all-target-libgcc && \
    make install-gcc && \
    make install-target-libgcc && \
    cd / && rm -rf /build-gcc /gcc-${GCC_VERSION}
RUN apt-get -y install nasm grub-pc-bin xorriso python3 gdb git libtool
ENV PYTHONUNBUFFERED=1