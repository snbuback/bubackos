FROM ubuntu:18.04
ARG LOCALE=en_GB.UTF-8
ARG BINUTILS_VERSION
ARG GCC_VERSION
ARG NEWLIB_VERSION
ARG SYSROOT
ARG CROSS_TRIPLE
ENV PYTHONUNBUFFERED=1
ENV BUILD_DIR=/build-tools
ENV CROSS_TRIPLE=${CROSS_TRIPLE}

# setup default locale
RUN echo -n "LC_ALL=${LOCALE}\n\
LANG=${LOCALE}\n\
LC_CTYPE=${LOCALE}\n\
LC_COLLATE=${LOCALE}\n" > /etc/default/locale && echo "${LOCALE} UTF-8" > /etc/locale.gen
ENV LC_ALL=${LOCALE} LANG=${LOCALE} LANGUAGE=${LOCALE}

# Upgrade and requirements
RUN apt-get -qq update && \
    apt-get -qq -y full-upgrade && \
    apt-get -qq -y install locales bash curl wget pkg-config build-essential make automake autogen \
        tar xz-utils bzip2 gzip file rsync sed vim binutils gcc nasm grub-pc-bin xorriso python3 python \
        gdb git libtool cmake automake1.11 autoconf2.64 gawk ruby qemu-system-x86 tmux lcov ninja-build && \
    rm -rf /var/lib/apt/lists/*


ADD tools/build-gcc.sh tools/build-binutils.sh tools/build-newlib.sh /tools/
RUN cd /tools && \
    ./build-binutils.sh ${BINUTILS_VERSION} && \
    ./build-gcc.sh ${GCC_VERSION} && \
    ./build-newlib.sh ${GCC_VERSION} ${NEWLIB_VERSION} && \
    rm -rf ${BUILD_DIR}

RUN curl -SsL https://github.com/oclint/oclint/releases/download/v0.13.1/oclint-0.13.1-x86_64-linux-4.4.0-112-generic.tar.gz | tar -zxv -C /usr/local
ENV PATH=$PATH:/usr/local/oclint-0.13.1/bin

