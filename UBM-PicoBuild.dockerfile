FROM ubuntu:22.04
RUN apt-get update && apt-get install -y --no-install-recommends \
	software-properties-common \
	build-essential cmake git wget ca-certificates python3 \
	libusb-1.0-0-dev libudev-dev \
	&& rm -rf /var/lib/apt/lists/*

RUN add-apt-repository universe && apt-get update && apt-get install -y --no-install-recommends \
	gcc-arm-none-eabi libstdc++-arm-none-eabi-newlib binutils-arm-none-eabi libnewlib-arm-none-eabi gdb-arm-none-eabi \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone --depth 1 --recurse-submodules https://github.com/raspberrypi/pico-sdk.git /opt/pico-sdk
RUN cd /opt/pico-sdk && git submodule update --init --recursive
ENV PICO_SDK_PATH=/opt/pico-sdk
WORKDIR /work
ENTRYPOINT ["/bin/bash"]