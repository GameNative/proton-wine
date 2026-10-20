#!/bin/sh

# wine-tools (winebuild, winegcc, ...) is built here, before the per-arch build
# steps apply android/patches, so patches to tools/ are applied in this step.
TOOLS_PATCHES="
  tools/tools_winebuild_arm64ec_x86_64_asm.patch
  tools/tools_winebuild_spec32_c_import_thunks.patch
"

for patch in $TOOLS_PATCHES; do
  if git apply --reverse --check ./android/patches/$patch 2>/dev/null; then
    echo "Already applied: $patch"
  else
    git apply ./android/patches/$patch || exit 1
  fi
done

mkdir wine-tools
cd wine-tools
../configure --without-x --without-gstreamer --without-vulkan --without-wayland
make -j$(nproc) __tooldeps__ nls/all
