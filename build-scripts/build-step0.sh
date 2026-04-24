#!/bin/sh
git apply "./android/patches/server_protocol.def.patch"
./autogen.sh

mkdir wine-tools
cd wine-tools
../configure --without-x --without-gstreamer --without-vulkan --without-wayland
make -j$(nproc) __tooldeps__ nls/all