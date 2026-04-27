#!/bin/bash

export ARCH="aarch64"
export WIN_ARCH="arm64ec,aarch64,i386"
export OUTPUT_DIR="$HOME/compiled-files-aarch64"

export deps="$HOME/termuxfs/aarch64/data/data/com.termux/files/usr"
export RUNTIME_PATH="/data/data/com.termux/files/usr"
export install_dir=$deps/../opt/wine

#export TOOLCHAIN="$HOME/Android/android-ndk-r27d/toolchains/llvm/prebuilt/linux-x86_64/bin"
export TOOLCHAIN="$HOME/Android/Sdk/ndk/27.3.13750724/toolchains/llvm/prebuilt/linux-x86_64/bin"
export LLVM_MINGW_TOOLCHAIN="$HOME/toolchains/llvm-mingw-20250920-ucrt-ubuntu-22.04-x86_64/bin"
export TARGET=aarch64-linux-android28
export PATH=$LLVM_MINGW_TOOLCHAIN:$PATH

export CC=$TOOLCHAIN/$TARGET-clang
export AS=$CC
export CXX=$TOOLCHAIN/$TARGET-clang++
export AR=$TOOLCHAIN/llvm-ar
export LD=$TOOLCHAIN/ld
export RANLIB=$TOOLCHAIN/llvm-ranlib
export STRIP=$TOOLCHAIN/llvm-strip
export DLLTOOL=$LLVM_MINGW_TOOLCHAIN/llvm-dlltool

export PKG_CONFIG_LIBDIR=$deps/lib/pkgconfig:$deps/share/pkgconfig
export ACLOCAL_PATH=$deps/lib/aclocal:$deps/share/aclocal
export CPPFLAGS="-I$deps/include --sysroot=$TOOLCHAIN/../sysroot"

export C_OPTS="-Wno-declaration-after-statement -Wno-implicit-function-declaration -Wno-int-conversion"
export CFLAGS=$C_OPTS
export CXXFLAGS=$C_OPTS
export LDFLAGS="-L$deps/lib -Wl,-rpath=$RUNTIME_PATH/lib"

export FREETYPE_CFLAGS="-I$deps/include/freetype2"
export PULSE_CFLAGS="-I$deps/include/pulse"
export PULSE_LIBS="-L$deps/lib/pulseaudio -lpulse"
export SDL2_CFLAGS="-I$deps/include/SDL2"
export SDL2_LIBS="-L$deps/lib -lSDL2"
export FONTCONFIG_LIBS="-L$deps/lib -lfontconfig -lfreetype -lexpat"
export X_CFLAGS="-I$deps/include/X11"
export X_LIBS="-landroid-sysvshm"
export GSTREAMER_CFLAGS="-I$deps/include/gstreamer-1.0 -I$deps/include/glib-2.0 -I$deps/lib/glib-2.0/include -I$deps/glib-2.0/include -I$deps/lib/gstreamer-1.0/include"
export GSTREAMER_LIBS="-L$deps/lib -lgstgl-1.0 -lgstapp-1.0 -lgstvideo-1.0 -lgstaudio-1.0 -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lgsttag-1.0 -lgstbase-1.0 -lgstreamer-1.0"
export FFMPEG_CFLAGS="-I$deps/include/libavutil -I$deps/include/libavcodec -I$deps/include/libavformat"
export FFMPEG_LIBS="-L$deps/lib -lavutil -lavcodec -lavformat"

for arg in "$@"
do
  if [ "$arg" == "--enable-16kb-pages" ];
  then
    echo "Enabling 16KB page size support..."
    export TARGET=aarch64-linux-android35
    export C_OPTS="$C_OPTS -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES"
    export CFLAGS="$C_OPTS"
    export CXXFLAGS="$C_OPTS"
    export LDFLAGS="$LDFLAGS -Wl,-z,max-page-size=16384"
    echo "16KB page size support enabled"
  fi

  if [ "$arg" == "--build-sysvshm" ];
  then
    # Build android_sysvshm library
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

    if [ -d "$PROJECT_ROOT/android/android_sysvshm" ]; then
        echo "Building android_sysvshm library..."
        cd "$PROJECT_ROOT/android/android_sysvshm"
        ./build-aarch64.sh
        if [ $? -eq 0 ]; then
            echo "android_sysvshm built successfully"
            # Copy the library to deps/lib for linking
            mkdir -p "$deps/lib"
            cp build-aarch64/libandroid-sysvshm.so "$deps/lib/"
            echo "Copied libandroid-sysvshm.so to $deps/lib/"
        else
            echo "Warning: android_sysvshm build failed"
        fi
        cd "$PROJECT_ROOT"
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
      --without-xcursor \
      --without-xfixes \
      --without-xinerama \
      --without-xrandr \
      --without-xrender \
      --without-xshape \
      --with-xshm \
      --without-xxf86vm

    echo "Applying patches..."

    PATCHES=(
      # core patches
      "dlls_advapi32_advapi.c.patch"
      "dlls_amd_ags_x64_unixlib.c.patch"

      # dns
      "dlls_dnsapi_libresolv.c.patch"
      "dlls_dnsapi_record.c.patch"

      # midi
      "dlls_midimap_Makefile.in.patch"
      "dlls_midimap_midimap.c.patch"

      # nsiproxy
	  "dlls_nsiproxy.sys_nsi_common.h.patch"
      "dlls_nsiproxy.sys_ip.c.patch"
      "dlls_nsiproxy.sys_ndis.c.patch"

      # ntdll
      "dlls_ntdll_Makefile.in.patch"
      "dlls_ntdll_unix_fsync.c.patch"
      "dlls_ntdll_unix_loader.c.patch"
      "dlls_ntdll_unix_server.c.patch"
      "dlls_ntdll_unix_sync.c.patch"
      "dlls_ntdll_unix_virtual.c.patch"
	  "dlls_ntdll_unix_signal_x86_64.c.patch"
	  
	  # opengl32
	  "dlls_opengl32_unix_wgl.c.patch"

      # user32 / clipboard
      "dlls_user32_Makefile.in.patch"
      "dlls_win32u_clipboard.c.patch"

      # drivers
      "dlls_winebus.sys_bus_sdl.c.patch"
      "dlls_winepulse.drv_pulse.c.patch"

      # winex11
      "dlls_winex11.drv_bitblt.c.patch"
      "dlls_winex11.drv_keyboard.c.patch"
      "dlls_winex11.drv_mouse.c.patch"
      "dlls_winex11.drv_opengl.c.patch"
      "dlls_winex11.drv_window.c.patch"
      "dlls_winex11.drv_x11drv.h.patch"
      "dlls_winex11.drv_x11drv_main.c.patch"

      # wow64
      # "dlls_wow64_process.c.patch"
      "dlls_wow64_syscall.c.patch"

      # loader
      "loader_preloader.c.patch"

      # programs
      "programs_explorer_desktop.c.patch"
      "programs_wineboot_wineboot.c.patch"
      "programs_winebrowser_Makefile.in.patch"
      "programs_winebrowser_main.c.patch"
      "programs_winemenubuilder_winemenubuilder.c.patch"

      # server
      "server_Makefile.in.patch"
      "server_fsync.c.patch"
      "server_inproc_sync.c.patch"
      "server_main.c.patch"
      # "server_protocol.def.patch"
      "server_thread.c.patch"
      "server_unicode.c.patch"
	  
	  # esync
	  "dlls_ntdll_unix_esync.c.patch"
	  "dlls_ntdll_unix_esync.h.patch"
	  "dlls_ntdll_unix_sync.h.patch"
	  "server_esync.c.patch"
	  "server_esync.h.patch"
    )

    for patch in "${PATCHES[@]}"; do
      echo "----------------------------------------"
      echo "Applying: $patch"

      if git apply --check "./android/patches/$patch" 2>/dev/null; then
        if git apply "./android/patches/$patch"; then
          echo "SUCCESS: $patch applied"
        else
          echo "FAILED: error applying $patch"
        fi
      else
        echo "SKIPPED: $patch does not apply cleanly"
      fi
    done

    echo "----------------------------------------"
    echo "Done applying patches."
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
    echo "Copying files..."
    cp -r $install_dir/bin/wine* $OUTPUT_DIR/bin
    cp -r $install_dir/bin/reg* $OUTPUT_DIR/bin
    cp -r $install_dir/bin/msi* $OUTPUT_DIR/bin
    cp -r $install_dir/bin/notepad $OUTPUT_DIR/bin
    cp -r $install_dir/lib/wine  $OUTPUT_DIR/lib
    cp -r $install_dir/share/wine  $OUTPUT_DIR/share
	# symlinking wine binaries to $install_dir/bin
    ln -sf ../lib/wine/aarch64-unix/wine "$install_dir/bin/wine"
    ln -sf ../lib/wine/aarch64-unix/wine "$OUTPUT_DIR/bin/wine"
    ln -sf ../lib/wine/aarch64-unix/wine-preloader "$OUTPUT_DIR/bin/wine-preloader"
    ln -sf ../lib/wine/aarch64-unix/wine-preloader "$install_dir/bin/wine-preloader"
    echo "Wine loader symlinks:"
    ls -la "$OUTPUT_DIR/bin/wine" "$OUTPUT_DIR/bin/wine-preloader"
  fi
done
