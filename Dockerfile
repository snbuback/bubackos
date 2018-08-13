FROM debian:stretch
ARG LOCALE=en_GB.UTF-8
ARG BINUTILS_VERSION
ARG GCC_VERSION
ARG SYSROOT
ARG CROSS_TRIPLE
ENV PYTHONUNBUFFERED=1
ENV BUILD_DIR=/build-tools
ENV CROSS_TRIPLE=${CROSS_TRIPLE}

# Upgrade and requirements
RUN apt-get -qq update && \
    apt-get -qq -y upgrade && \
    apt-get -qq -y install locales bash curl wget pkg-config build-essential make automake autogen \
        tar xz-utils bzip2 gzip file rsync sed vim binutils gcc nasm grub-pc-bin xorriso python3 \
        gdb git libtool cmake automake1.11 autoconf2.64 gawk ruby qemu-system-x86 tmux

# setup default locale
RUN echo -n "LC_ALL=${LOCALE}\n\
LANG=${LOCALE}\n\
LC_CTYPE=${LOCALE}\n\
LC_COLLATE=${LOCALE}\n" > /etc/default/locale && echo "${LOCALE} UTF-8" > /etc/locale.gen && locale-gen ${LOCALE}
ENV LC_ALL=${LOCALE} LANG=${LOCALE} LANGUAGE=${LOCALE}

ADD tools/build-gcc.sh tools/build-binutils.sh /tools/
RUN cd /tools && \
    ./build-binutils.sh ${BINUTILS_VERSION} && \
    ./build-gcc.sh ${GCC_VERSION} && \
    rm -rf ${BUILD_DIR}
