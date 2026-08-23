This is the Android SysVSHM used in Winlator based on https://github.com/pelya/android-shmem

## Overview

This library provides System V shared memory (SysV SHM) support for Android, which lacks native support for these IPC mechanisms. It implements the standard `shmget()`, `shmat()`, `shmdt()`, and `shmctl()` functions by communicating with a server process via Unix domain sockets.

## Build Instructions

### Using the build script (recommended):
```bash
./build-x86_64.sh
```

### Using Make:
```bash
make
```

### Manual build:
```bash
export TOOLCHAIN=$HOME/Android/android-ndk-r28c/toolchains/llvm/prebuilt/linux-x86_64/bin
export TARGET=x86_64-linux-android28
$TOOLCHAIN/$TARGET-clang -Wall -std=gnu99 -shared -fPIC -I. -o build-x86_64/libandroid-sysvshm.so android_sysvshm.c
```

## Integration

The library is automatically built when running the main Wine build script:
```bash
cd ../build-scripts
./build-step-x86_64.sh --configure
```

The compiled library will be copied to `$deps/lib/libandroid-sysvshm.so` for linking with Wine.

## Usage

The library requires the `ANDROID_SYSVSHM_SERVER` environment variable to be set to the path of the Unix domain socket for the SysV SHM server.

## API

- `int shmget(key_t key, size_t size, int flags)` - Get shared memory segment
- `void* shmat(int shmid, const void* shmaddr, int shmflg)` - Attach shared memory segment
- `int shmdt(const void* shmaddr)` - Detach shared memory segment
- `int shmctl(int shmid, int cmd, struct shmid_ds* buf)` - Control shared memory segment

## Build dependencies

    $ dpkg --add-architecture armhf
    $ apt install build-essential make cmake g++-arm-linux-gnueabihf
