#!/bin/bash
# Builds the x86-64 Linux (glibc) Wine that runs under a Linux emulator as a client of the
# ARM64EC build's wineserver. Both builds must come from the same commit: they share the
# server protocol.
#
# Steps: --configure (apply patches, configure), --build, --install, --package.
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${BUILD_DIR:-$HOME/build-x86_64-linux}"
OUTPUT_DIR="${OUTPUT_DIR:-$HOME/compiled-files-x86_64-linux}"
LLVM_MINGW_TOOLCHAIN="${LLVM_MINGW_TOOLCHAIN:-$HOME/toolchains/llvm-mingw-20250920-ucrt-ubuntu-22.04-x86_64/bin}"
export PATH="$LLVM_MINGW_TOOLCHAIN:$PATH"

if command -v ccache >/dev/null 2>&1; then
  export CCACHE_DIR="${CCACHE_DIR:-$HOME/.ccache}"
  ccache -M 3G >/dev/null 2>&1 || true
  mkdir -p "$HOME/ccache-bin"
  ln -sf "$(command -v ccache)" "$HOME/ccache-bin/clang"
  ln -sf "$(command -v ccache)" "$HOME/ccache-bin/clang++"
  export PATH="$HOME/ccache-bin:$PATH"
  export CC="ccache gcc"
  export CXX="ccache g++"
fi

export CFLAGS="-g0 -O2 -march=x86-64 -msse3 -mfpmath=sse"
export CXXFLAGS="$CFLAGS"
export CROSSCFLAGS="-g0 -O2"

# Patches of the Android builds that change the server protocol and are needed here as they are.
PROTOCOL_PATCHES=(
  "common/dlls_ntdll_unix_socket_c_recv_fastpath.patch"
)

# The protocol change of arm64ec/server_hybrid_x64_clients.patch is in
# x86_64-linux/ntdll_server_native_machine.patch, with the client side.
PATCHES=(
  "x86_64-linux/ntdll_fixed_alloc_above_host_limit.patch"
  "x86_64-linux/ntdll_seccomp_small_address_space.patch"
  "x86_64-linux/ntdll_server_native_machine.patch"
  "x86_64-linux/ntdll_own_builtins_for_arm64_images.patch"
  "x86_64-linux/ntdll_process_routing.patch"
  "x86_64-linux/ntdll_x64_dll_for_arm64ec_native.patch"
  "x86_64-linux/ntdll_image_sections_noexec_fs.patch"
  "x86_64-linux/ntdll_process_routing_i386.patch"
  "x86_64-linux/ntdll_process_routing_il_only.patch"
  "x86_64-linux/ntdll_prefix_server_dir.patch"
  "x86_64-linux/ntdll_session_desktop.patch"
  "x86_64-linux/advapi32_user_name.patch"
  "x86_64-linux/ntdll_64bit_preloader.patch"
  "x86_64-linux/ntdll_reserve_unreserved_low.patch"
)

# Every arm64ec-list patch that touches the protocol must be accounted for here, or the two
# builds stop understanding each other.
check_protocol_patches() {
  local known=" ${PROTOCOL_PATCHES[*]} arm64ec/server_hybrid_x64_clients.patch "
  local missing=0 patch
  while read -r patch; do
    [ -f "$PROJECT_ROOT/android/patches/$patch" ] || continue
    grep -q '^+++ b/server/protocol.def' "$PROJECT_ROOT/android/patches/$patch" || continue
    if [[ "$known" != *" $patch "* ]]; then
      echo "error: $patch changes server/protocol.def and is not in PROTOCOL_PATCHES of build-step-x86_64-linux.sh" >&2
      missing=1
    fi
  done < <(awk '/^    PATCHES=\(/{f=1;next} f&&/^    \)/{f=0} f' "$SCRIPT_DIR/build-step-arm64ec.sh" \
             | grep -v '^[[:space:]]*#' | grep -o '"[^"]*"' | tr -d '"')
  return $missing
}

for arg in "$@"; do
  case "$arg" in
  --configure)
    cd "$PROJECT_ROOT"
    check_protocol_patches
    echo "Applying patches..."
    for patch in "${PROTOCOL_PATCHES[@]}" "${PATCHES[@]}"; do
      git apply "./android/patches/$patch"
    done
    grep 'define SERVER_PROTOCOL_VERSION' include/wine/server_protocol.h
    sha256sum server/protocol.def

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    "$PROJECT_ROOT/configure" \
      --enable-win64 \
      --with-mingw=clang \
      --prefix="$OUTPUT_DIR" \
      --disable-tests \
      --disable-winemenubuilder \
      --disable-lsteamclient \
      --without-unwind \
      --with-x \
      --with-freetype \
      --with-fontconfig \
      --with-gnutls \
      --with-pulse \
      --with-vulkan \
      --without-alsa \
      --without-capi \
      --without-cups \
      --without-gphoto \
      --without-gstreamer \
      --without-krb5 \
      --without-netapi \
      --without-opencl \
      --without-oss \
      --without-pcap \
      --without-pcsclite \
      --without-sane \
      --without-usb \
      --without-v4l2 \
      --without-wayland
    ;;
  --build)
    cd "$BUILD_DIR"
    make -j"$(nproc)"
    ;;
  --install)
    cd "$BUILD_DIR"
    rm -rf "$OUTPUT_DIR"
    make install -j"$(nproc)"
    find "$OUTPUT_DIR/lib/wine/x86_64-unix" -type f -exec strip --strip-unneeded {} + 2>/dev/null || true
    # ntdll starts a 64-bit process through i386-unix/wine64-preloader, where a build with both
    # architectures has its 64-bit loader; this 64-bit-only build gets the same layout.
    mkdir -p "$OUTPUT_DIR/lib/wine/i386-unix"
    cp -p "$OUTPUT_DIR/lib/wine/x86_64-unix/wine64" "$OUTPUT_DIR/lib/wine/x86_64-unix/wine64-preloader" "$OUTPUT_DIR/lib/wine/i386-unix/"
    ;;
  --package)
    # Only what a 64-bit process loads: the wineserver, the tools and the documentation stay out.
    PACKAGE_NAME="${PACKAGE_NAME:-proton-11.0-3-x86_64-linux.tar.zst}"
    cd "$OUTPUT_DIR"
    for f in lib/wine/x86_64-unix/wine64 lib/wine/i386-unix/wine64 lib/wine/i386-unix/wine64-preloader lib/wine/x86_64-unix/ntdll.so lib/wine/x86_64-windows/ntdll.dll; do
      [ -f "$f" ] || { echo "error: $f is missing from the build" >&2; exit 1; }
    done
    # The device's x86-64 root file system holds a fixed set of libraries; a module that needs
    # another one cannot load there.
    missing=0
    for f in lib/wine/x86_64-unix/*; do
      for lib in $(readelf -d "$f" 2>/dev/null | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p'); do
        [ -e "lib/wine/x86_64-unix/$lib" ] && continue
        grep -qxF "$lib" "$PROJECT_ROOT/android/x86_64-linux-runtime-libs.txt" && continue
        echo "error: $f needs $lib, which is not in android/x86_64-linux-runtime-libs.txt" >&2
        missing=1
      done
    done
    [ "$missing" -eq 0 ] || exit 1
    grep 'define SERVER_PROTOCOL_VERSION' "$PROJECT_ROOT/include/wine/server_protocol.h" | awk '{print $3}' > share/wine/server-protocol-version
    git -C "$PROJECT_ROOT" rev-parse HEAD > share/wine/build-commit
    tar -I 'zstd -T0 -19' -cf "$PROJECT_ROOT/$PACKAGE_NAME" lib/wine/x86_64-unix lib/wine/i386-unix lib/wine/x86_64-windows share/wine
    ls -la "$PROJECT_ROOT/$PACKAGE_NAME"
    ;;
  esac
done
