#!/bin/sh
set -e

APPLY_ANDROID_PATCH=0

while [ $# -gt 0 ]; do
    case "$1" in
        --aarch64)
            APPLY_ANDROID_PATCH=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--aarch64]"
            exit 1
            ;;
    esac
done

tools/make_requests
tools/make_specfiles
dlls/winevulkan/make_vulkan -x vk.xml -X video.xml

if [ $APPLY_ANDROID_PATCH -eq 1 ]; then
    echo "Applying Android patch to configure.ac..."
    patch -p1 < android/patches/test-bylaws/configure_ac.patch
fi

autoreconf -ifv
rm -rf autom4te.cache

echo "Now run ./configure"
