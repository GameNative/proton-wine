#!/bin/bash

export TOOLCHAIN=$HOME/Android/android-ndk-r27d/toolchains/llvm/prebuilt/linux-x86_64/bin
export TARGET=x86_64-linux-android28

export CC="$TOOLCHAIN/$TARGET-clang"
export AR="$TOOLCHAIN/llvm-ar"
export RANLIB="$TOOLCHAIN/llvm-ranlib"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="$SCRIPT_DIR/build-x86_64"

mkdir -p "$OUTPUT_DIR"

echo "Building android_sysvshm for x86_64..."

$CC -Wall -std=gnu99 -shared -fPIC \
    -I"$SCRIPT_DIR" \
    -o "$OUTPUT_DIR/libandroid-sysvshm.so" \
    "$SCRIPT_DIR/android_sysvshm.c"

if [ $? -eq 0 ]; then
    echo "Build successful! Output: $OUTPUT_DIR/libandroid-sysvshm.so"
    #ls -lh "$OUTPUT_DIR/libandroid-sysvshm.so"
else
    echo "Build failed!"
    exit 1
fi
