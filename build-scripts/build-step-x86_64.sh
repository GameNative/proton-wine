#!/bin/bash

export ARCH="x86_64"
export WIN_ARCH="x86_64,i386"
export OUTPUT_DIR="$HOME/compiled-files-x86_64"

export deps="$HOME/termuxfs/x86_64/data/data/com.termux/files/usr"
export RUNTIME_PATH="/data/data/com.termux/files/usr"
export install_dir=$deps/../opt/wine

#export TOOLCHAIN="$HOME/Android/android-ndk-r27d/toolchains/llvm/prebuilt/linux-x86_64/bin"
export TOOLCHAIN="$HOME/Android/Sdk/ndk/27.3.13750724/toolchains/llvm/prebuilt/linux-x86_64/bin"
export LLVM_MINGW_TOOLCHAIN="$HOME/toolchains/llvm-mingw-20250920-ucrt-ubuntu-22.04-x86_64/bin"
export TARGET=x86_64-linux-android28
export PATH=$LLVM_MINGW_TOOLCHAIN:$PATH

# ccache — wrap clang so both the host-side and cross compiler calls are cached.
# ccache symlinks are placed first on PATH, so Wine's cross-compiler calls go through ccache too.
if command -v ccache >/dev/null 2>&1; then
  export CCACHE_DIR="${CCACHE_DIR:-$HOME/.ccache}"
  ccache -M 3G >/dev/null 2>&1 || true
  mkdir -p "$HOME/ccache-bin"
  ln -sf "$(command -v ccache)" "$HOME/ccache-bin/clang"
  ln -sf "$(command -v ccache)" "$HOME/ccache-bin/clang++"
  export PATH="$HOME/ccache-bin:$PATH"
  export CC="ccache $TOOLCHAIN/$TARGET-clang"
  export CXX="ccache $TOOLCHAIN/$TARGET-clang++"
else
  export CC=$TOOLCHAIN/$TARGET-clang
  export CXX=$TOOLCHAIN/$TARGET-clang++
fi
export AS=$TOOLCHAIN/$TARGET-clang
export AR=$TOOLCHAIN/llvm-ar
export LD=$TOOLCHAIN/ld
export RANLIB=$TOOLCHAIN/llvm-ranlib
export STRIP=$TOOLCHAIN/llvm-strip
export DLLTOOL=$LLVM_MINGW_TOOLCHAIN/llvm-dlltool

# Cross-compiling: wine's WINE_CHECK_HOST_TOOL deliberately skips the non-prefixed
# pkg-config fallback, so point PKG_CONFIG at the host tool explicitly.
export PKG_CONFIG="${PKG_CONFIG:-$(command -v pkg-config)}"
export PKG_CONFIG_LIBDIR=$deps/lib/pkgconfig:$deps/share/pkgconfig
export ACLOCAL_PATH=$deps/lib/aclocal:$deps/share/aclocal
export CPPFLAGS="-I$deps/include --sysroot=$TOOLCHAIN/../sysroot"

# -g0 + post-install llvm-strip keep the packaged tree small; ANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES
# + max-page-size=16384 give 16KB page support on a single SDK 28 target.
export C_OPTS="-g0 -O2 -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES -march=x86-64 -mtune=generic -Wno-declaration-after-statement -Wno-implicit-function-declaration -Wno-int-conversion"
export CFLAGS=$C_OPTS
export CXXFLAGS=$C_OPTS
export CROSSCFLAGS="-g0 -O2"
export LDFLAGS="-L$deps/lib -Wl,-rpath=$RUNTIME_PATH/lib -Wl,-z,max-page-size=16384"

export FREETYPE_CFLAGS="-I$deps/include/freetype2"
export PULSE_CFLAGS="-I$deps/include/pulse"
export PULSE_LIBS="-L$deps/lib/pulseaudio -lpulse"
export SDL2_CFLAGS="-I$deps/include/SDL2"
export SDL2_LIBS="-L$deps/lib -lSDL2"
export X_CFLAGS="-I$deps/include/X11"
export X_LIBS=""
export GSTREAMER_CFLAGS="-I$deps/include/gstreamer-1.0 -I$deps/include/glib-2.0 -I$deps/lib/glib-2.0/include -I$deps/glib-2.0/include -I$deps/lib/gstreamer-1.0/include"
export GSTREAMER_LIBS="-L$deps/lib -lgstgl-1.0 -lgstapp-1.0 -lgstvideo-1.0 -lgstaudio-1.0 -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lgsttag-1.0 -lgstbase-1.0 -lgstreamer-1.0"
export FFMPEG_CFLAGS="-I$deps/include/libavutil -I$deps/include/libavcodec -I$deps/include/libavformat"
export FFMPEG_LIBS="-L$deps/lib -lavutil -lavcodec -lavformat"

for arg in "$@"
do
  if [ "$arg" == "--build-sysvshm" ];
  then
    # Build android_sysvshm library
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

    if [ -d "$PROJECT_ROOT/android/android_sysvshm" ]; then
        echo "Building android_sysvshm library..."
        cd "$PROJECT_ROOT/android/android_sysvshm"
        ./build-x86_64.sh
        if [ $? -eq 0 ]; then
            echo "android_sysvshm built successfully"
            # Copy the library to deps/lib for linking
            mkdir -p "$deps/lib"
            cp build-x86_64/libandroid-sysvshm.so "$deps/lib/"
            echo "Copied libandroid-sysvshm.so to $deps/lib/"
        else
            echo "Warning: android_sysvshm build failed"
        fi
        cd "$PROJECT_ROOT"
    fi
  fi

  if [ "$arg" == "--build-ntsync-android" ];
  then
    # Build libntsync_android.a (userspace ntsync) from the sibling project
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
    NTSYNC_DIR="${NTSYNC_ANDROID_DIR:-$PROJECT_ROOT/../ntsync-android}"

    if [ -d "$NTSYNC_DIR" ]; then
        echo "Building ntsync-android library..."
        "$NTSYNC_DIR/build-scripts/build-android.sh" --build
        if [ $? -eq 0 ]; then
            echo "ntsync-android built successfully"
            mkdir -p "$deps/lib"
            # Static archive: ntdll/wineserver link it in, no runtime .so needed.
            cp "$NTSYNC_DIR/target/x86_64-linux-android/release/libntsync_android.a" "$deps/lib/"
            rm -f "$deps/lib/libntsync_android.so"
            echo "Copied libntsync_android.a (x86_64) to $deps/lib/"
            # The archive is statically linked into ntdll/wineserver, but make
            # does not track it as a dependency; force a relink.
            rm -f "$PROJECT_ROOT/dlls/ntdll/ntdll.so" "$PROJECT_ROOT/server/wineserver" "$PROJECT_ROOT/server/wineserver64" 2>/dev/null
        else
            echo "Warning: ntsync-android build failed"
        fi
    else
        echo "Warning: ntsync-android project not found at $NTSYNC_DIR"
    fi
  fi

  if [ "$arg" == "--configure" ];
  then
    ./configure \
      --enable-archs=$WIN_ARCH \
      --host=$TARGET \
      --prefix $install_dir \
      --bindir $install_dir/bin \
      --libdir $install_dir/lib \
      --exec-prefix $install_dir \
      --with-mingw=clang \
      --with-wine-tools=./wine-tools \
      --enable-win64 \
      --disable-win16 \
      --enable-nls \
      --disable-amd_ags_x64 \
      --enable-wineandroid_drv=no \
      --disable-tests \
      --with-alsa \
      --without-capi \
      --without-coreaudio \
      --without-cups \
      --without-dbus \
      --without-ffmpeg \
      --with-fontconfig \
      --with-freetype \
      --without-gcrypt \
      --without-gettext \
      --with-gettextpo=no \
      --without-gphoto \
      --with-gnutls \
      --without-gssapi \
      --with-gstreamer \
      --without-inotify \
      --without-krb5 \
      --without-netapi \
      --without-opencl \
      --with-opengl \
      --without-oss \
      --without-pcap \
      --without-pcsclite \
      --without-piper \
      --with-pthread \
      --with-pulse \
      --without-sane \
      --with-sdl \
      --without-udev \
      --without-unwind \
      --without-usb \
      --without-v4l2 \
      --without-vosk \
      --with-vulkan \
      --without-wayland \
      --without-xcomposite \
      --without-xfixes \
      --without-xinerama \
      --without-xrandr \
      --without-xrender \
      --without-xshape \
      --without-xshm \
      --without-xxf86vm

    echo "Applying patches..."

    PATCHES=(
      # android network patch
      "common/dlls_dnsapi_libresolv_c.patch"
      "common/dlls_dnsapi_record_c.patch"
      "common/dlls_nsiproxy_sys_ip_c.patch"
      "common/dlls_nsiproxy_sys_ndis_c.patch"
      "common/dlls_nsiproxy_sys_nsi_common_h.patch"
      "common/dlls_user32_makefile_in.patch"
      "common/dlls_ws2_32_socket_c.patch"
      "common/server_token_c.patch"
      "common/server_unicode_c.patch"

      # midi support
      "common/midi_support.patch"

      # sdl patch
      "common/dlls_winebus_sys_bus_sdl_c.patch"

      # shm_utils
      "common/dlls_ntdll_unix_fsync_c.patch"
      "common/server_fsync_c.patch"

      # ntsync (userspace ntsync via libntsync_android)
      "common/dlls_ntdll_makefile_in.patch"
      "common/dlls_ntdll_unix_sync_c.patch"
      "common/server_makefile_in.patch"
      "common/server_inproc_sync_c.patch"
      "common/server_thread_c.patch"
      "common/server_process_c.patch"

      # winex11
      "common/dlls_winex11_drv_bitblt_c.patch"
      "common/dlls_winex11_drv_desktop_c.patch"
      "common/dlls_winex11_drv_keyboard_c.patch"
      "common/dlls_winex11_drv_mouse_c.patch"
      "common/dlls_winex11_drv_opengl_c.patch"
      "common/dlls_winex11_drv_window_c.patch"
      "common/dlls_winex11_drv_x11drv_h.patch"
      "common/dlls_winex11_drv_x11drv_main_c.patch"

      # address space patches
      "common/loader_preloader_c.patch"
      "x86_64/dlls_ntdll_unix_virtual_c.patch"

      # syscall Patches
      "x86_64/dlls_ntdll_unix_signal_x86_64_c.patch"

      # pulse Patches
      "common/dlls_winepulse_drv_pulse_c.patch"

      # opengl32 patches
      "common/dlls_opengl32_unix_wgl_c.patch"

      # desktop patches
      "common/programs_explorer_desktop_c.patch"

      # path patches
      "common/dlls_ntdll_unix_server_c.patch"

      # winlator patches
      "common/dlls_amd_ags_x64_unixlib_c.patch"

      # shortcut patch
      "common/programs_winemenubuilder_winemenubuilder_c.patch"

      # xuser patches
      "common/dlls_advapi32_advapi_c.patch"

      # browser patches
      "common/programs_winebrowser_makefile_in.patch"
      "common/programs_winebrowser_main_c.patch"

      # clipboard patches
      "common/dlls_user32_clipboard_c.patch"
      "common/dlls_win32u_clipboard_c.patch"

      # loader patch
      "x86_64/dlls_ntdll_unix_loader_c.patch"
    )

    for patch in "${PATCHES[@]}"; do
#      if git apply --check ./android/patches/$patch 2>/dev/null; then
        git apply ./android/patches/$patch
#      fi
    done
  fi

  if [ "$arg" == "--package-wcp" ]
  then
    # Package $OUTPUT_DIR as a GameNative/Winlator .wcp (xz-compressed tar).
    # Layout: profile.json, bin/, lib/, share/, prefixPack.txz.
    # Overrides: WCP_NAME, WCP_TYPE (Wine|Proton), WCP_VERSION_CODE, WCP_PREFIX_PACK
    # (path to a prebuilt prefixPack.txz; falls back to an empty one).
    WCP_NAME="${WCP_NAME:-proton-11.0-2-x86_64.wcp}"
    WCP_TYPE="${WCP_TYPE:-Proton}"
    ARCH_NAME="x86_64"
    WCP_VERSION_CODE="${WCP_VERSION_CODE:-1}"
    WCP_PREFIX_PACK="${WCP_PREFIX_PACK:-}"
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
    STAGING="$(mktemp -d)"
    trap 'rm -rf "$STAGING"' EXIT

    if [ ! -d "$OUTPUT_DIR/bin" ] || [ ! -d "$OUTPUT_DIR/lib/wine" ]; then
      echo "ERROR: $OUTPUT_DIR is not populated; run --install first."
      exit 1
    fi

    cp -a "$OUTPUT_DIR/bin" "$OUTPUT_DIR/lib" "$OUTPUT_DIR/share" "$STAGING/"

    if [ -z "$WCP_PREFIX_PACK" ] && [ -f "$PROJECT_ROOT/android/prefixPack-$ARCH_NAME.txz" ]; then
      WCP_PREFIX_PACK="$PROJECT_ROOT/android/prefixPack-$ARCH_NAME.txz"
    fi
    if [ -z "$WCP_PREFIX_PACK" ]; then
      PREFIX_PACK_URL="https://github.com/GameNative/bionic-prefix-files/raw/main/prefixPack-$ARCH_NAME-11.txz"
      echo "Downloading prefixPack from $PREFIX_PACK_URL ..."
      if wget -q -O "$PROJECT_ROOT/android/prefixPack-$ARCH_NAME.txz" "$PREFIX_PACK_URL"; then
        WCP_PREFIX_PACK="$PROJECT_ROOT/android/prefixPack-$ARCH_NAME.txz"
      else
        rm -f "$PROJECT_ROOT/android/prefixPack-$ARCH_NAME.txz"
        echo "Warning: prefixPack download failed."
      fi
    fi
    if [ -n "$WCP_PREFIX_PACK" ] && [ -f "$WCP_PREFIX_PACK" ]; then
      cp "$WCP_PREFIX_PACK" "$STAGING/prefixPack.txz"
    else
      echo "Note: no prefixPack.txz found; packaging an empty one (GameNative will create the prefix on first launch)."
      mkdir -p "$STAGING/empty-prefix"
      tar -C "$STAGING/empty-prefix" -cJf "$STAGING/prefixPack.txz" --files-from /dev/null
      rm -rf "$STAGING/empty-prefix"
    fi

    cat > "$STAGING/profile.json" <<EOF
{
  "type": "$WCP_TYPE",
  "versionName": "11.0-2-$ARCH_NAME",
  "versionCode": $WCP_VERSION_CODE,
  "description": "Proton 11.0-2 $ARCH_NAME (bionic) — stock Valve + userspace ntsync + fsync + Android fixes. SDK 28 + 16KB pages. Needs a fresh $ARCH_NAME container.",
  "files": [],
  "wine": {
    "binPath": "bin",
    "libPath": "lib",
    "prefixPack": "prefixPack.txz"
  }
}
EOF

    out="$(dirname "$OUTPUT_DIR")/$WCP_NAME"
    rm -f "$out"
    tar -C "$STAGING" -cJf "$out" profile.json prefixPack.txz bin lib share
    rm -rf "$STAGING"
    trap - EXIT
    echo "WCP package: $out ($(du -m "$out" | cut -f1)MB)"
  fi

  if [ "$arg" == "--build" ]
  then
    echo "Building..."
    rm -rf $OUTPUT_DIR/bin
    rm -rf $OUTPUT_DIR/lib
    rm -rf $OUTPUT_DIR/share
    rm -rf $install_dir
    make -j$(nproc)
  fi

  if [ "$arg" == "--install" ]
  then
    echo "Installing..."
    mkdir -p $OUTPUT_DIR/bin
    mkdir -p $OUTPUT_DIR/lib
    mkdir -p $OUTPUT_DIR/share
    mkdir -p $install_dir
    make install -j$(nproc)
    cp -r $install_dir/bin/wine* $OUTPUT_DIR/bin
    cp -r $install_dir/bin/reg* $OUTPUT_DIR/bin
    cp -r $install_dir/bin/msi* $OUTPUT_DIR/bin
    cp -r $install_dir/bin/notepad $OUTPUT_DIR/bin
    cp -r $install_dir/lib/wine  $OUTPUT_DIR/lib
    cp -r $install_dir/share/wine  $OUTPUT_DIR/share

    # Strip the packaged binaries to shrink the tree. llvm-strip ($STRIP) is COFF-aware AND
    # handles ELF, so it strips both the PE DLLs/EXEs and the unix .so loaders. --strip-all keeps the
    # PE export directory + ELF .dynsym (so DLLs still resolve and .so still loads); falls back to
    # --strip-debug. Non-fatal per file so an unexpected format can never fail the build.
    echo "Stripping binaries with llvm-strip to shrink the tree..."
    before_mb=$(du -sm "$OUTPUT_DIR" 2>/dev/null | cut -f1)
    find "$OUTPUT_DIR/lib" "$OUTPUT_DIR/bin" -type f \
      \( -name '*.dll' -o -name '*.exe' -o -name '*.drv' -o -name '*.so' -o -name 'wine' -o -name 'wine-preloader' \) \
      -print0 2>/dev/null | while IFS= read -r -d '' f; do
        "$STRIP" --strip-all "$f" 2>/dev/null || "$STRIP" --strip-debug "$f" 2>/dev/null || true
      done
    after_mb=$(du -sm "$OUTPUT_DIR" 2>/dev/null | cut -f1)
    echo "OUTPUT tree: ${before_mb}MB -> ${after_mb}MB after strip."

    # symlinking wine binaries to $install_dir/bin
    ln -sf ../lib/wine/x86_64-unix/wine "$install_dir/bin/wine"
    ln -sf ../lib/wine/x86_64-unix/wine "$OUTPUT_DIR/bin/wine"
    ln -sf ../lib/wine/x86_64-unix/wine-preloader "$OUTPUT_DIR/bin/wine-preloader"
    ln -sf ../lib/wine/x86_64-unix/wine-preloader "$install_dir/bin/wine-preloader"
    echo "Wine loader symlinks:"
    ls -la "$OUTPUT_DIR/bin/wine" "$OUTPUT_DIR/bin/wine-preloader"
  fi
done
