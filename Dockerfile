FROM debian:stretch
ARG BINUTILS_VERSION
ARG GCC_VERSION
ARG NEWLIB_VERSION
ARG JERRYSCRIPT_VERSION
ARG SYSROOT
ARG CROSS_TRIPLE
ENV PYTHONUNBUFFERED=1
ENV BUILD_DIR=/build-tools
ENV CROSS_TRIPLE=${CROSS_TRIPLE}

# Upgrade and requirements
RUN apt-get -qq update && \
    apt-get -qq -y upgrade && \
    apt-get -qq -y install bash curl wget pkg-config build-essential make automake autogen \
        tar xz-utils bzip2 gzip file rsync sed vim binutils gcc nasm grub-pc-bin xorriso python3 \
        gdb git libtool cmake automake1.11 autoconf2.64

ADD tools/build-gcc.sh tools/build-binutils.sh tools/build-newlib.sh /tools/
RUN cd /tools && \
    ./build-binutils.sh ${BINUTILS_VERSION} && \
    ./build-gcc.sh ${GCC_VERSION} && \
    ./build-newlib.sh ${GCC_VERSION} ${NEWLIB_VERSION} && \
    rm -rf ${BUILD_DIR}

# jerryscript
ADD tools/crt0.S tools/build-jerryscript.sh tools/toolchain_bubackos.cmake /tools/
RUN cd /tools && \
    ./build-jerryscript.sh ${JERRYSCRIPT_VERSION} && \
    rm -rf ${BUILD_DIR}

# Improve
RUN apt-get -qq -y install gawk openjdk-8-jdk

# Initialize gradew
ENV GRADLE_OPTS=-Dorg.gradle.daemon=false
ADD settings.gradle gradlew /tools/builder/
ADD gradle /tools/builder/gradle/
RUN cd /tools/builder/ && chmod +x ./gradlew && ls -la ; ./gradlew -version