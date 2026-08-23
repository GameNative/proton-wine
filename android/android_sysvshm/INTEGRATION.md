# Android SysVSHM Integration Guide

## Integration Status: ✅ Complete

The android_sysvshm library has been successfully integrated into the Wine bleeding-edge project.

## What Was Done

1. **Created android_sysvshm directory structure**
   - `android_sysvshm/android_sysvshm.c` - Main implementation
   - `android_sysvshm/sys/shm.h` - Header file with SysV SHM definitions
   - `android_sysvshm/build-x86_64.sh` - Build script for x86_64 Android
   - `android_sysvshm/Makefile` - Alternative build using Make
   - `android_sysvshm/README.md` - Documentation

2. **Integrated into build system**
   - Modified `build-scripts/build-step-x86_64.sh` to automatically build android_sysvshm
   - Library is built before Wine configuration
   - Compiled library is copied to `$deps/lib/libandroid-sysvshm.so`

3. **Fixed compilation issues**
   - Added missing `string.h` header
   - Removed `-lpthread` flag (pthread is built into Android Bionic libc)
   - Fixed uninitialized variable warning

## Build Output

- **Library**: `android_sysvshm/build-x86_64/libandroid-sysvshm.so`
- **Size**: ~11KB
- **Architecture**: ELF 64-bit LSB shared object, x86-64
- **Target**: Android API 28 (x86_64-linux-android28)

## How to Build

### Standalone build:
```bash
cd android_sysvshm
./build-x86_64.sh
```

### Integrated with Wine build:
```bash
cd build-scripts
./build-step-x86_64.sh --configure
./build-step-x86_64.sh --build
./build-step-x86_64.sh --install
```

The android_sysvshm library will be automatically built during the configure step.

## Runtime Requirements

The library requires the `ANDROID_SYSVSHM_SERVER` environment variable to point to the Unix domain socket of the SysV SHM server process.

## Source

Based on: https://github.com/coffincolors/winlator/tree/cmod_bionic/android_sysvshm
Original implementation: https://github.com/pelya/android-shmem
