FROM ubuntu:24.04 as ubuntu

RUN <<EOF
dpkg --add-architecture i386
apt-get update
apt-get install -y \
   build-essential \
            git \
            wget \
            curl \
            unzip \
            flex \
            bison \
            gettext \
            autoconf \
            automake \
            libtool \
            pkg-config \
            mingw-w64 \
            gcc-multilib \
            g++-multilib \
            libfreetype6-dev \
            libfreetype6-dev:i386 \
            libpng-dev \
            libpng-dev:i386 \
            zlib1g-dev \
            zlib1g-dev:i386 \
            python3 \
            libpcap-dev:i386 \
            libudev-dev:i386 \
            dbus:i386 \
            libpulse-dev:i386 \
            ffmpeg:i386 \
            libsdl2-dev:i386 \
            fontconfig:i386 \
            libusb-1.0-0-dev:i386 \
            libv4l-dev:i386 \
            libavcodec-dev:i386 \
            libavformat-dev:i386 \
            libavfilter-dev:i386 \
            libswscale-dev:i386
EOF

RUN <<EOF
mkdir -p $HOME/termuxfs/aarch64
cd $HOME/termuxfs/aarch64
wget https://github.com/GameNative/termux-on-gha/releases/download/build-20260218/termuxfs-aarch64.tar
tar -xf termuxfs-aarch64.tar
ls -la $HOME/termuxfs/aarch64/

mkdir -p $HOME/termuxfs/x86_64
cd $HOME/termuxfs/x86_64
wget https://github.com/GameNative/termux-on-gha/releases/download/build-20260218/termuxfs-x86_64.tar
tar -xf termuxfs-x86_64.tar
ls -la $HOME/termuxfs/x86_64/
EOF

RUN <<EOF
mkdir -p $HOME/Android/Sdk/ndk
cd $HOME/Android/Sdk/ndk
wget https://dl.google.com/android/repository/android-ndk-r27d-linux.zip
unzip -q android-ndk-r27d-linux.zip
mv android-ndk-r27d 27.3.13750724
EOF

RUN <<EOF
mkdir -p $HOME/toolchains
cd $HOME/toolchains
wget https://github.com/bylaws/llvm-mingw/releases/download/20250920/llvm-mingw-20250920-ucrt-ubuntu-22.04-x86_64.tar.xz
tar -xf llvm-mingw-20250920-ucrt-ubuntu-22.04-x86_64.tar.xz
EOF

CMD ["/bin/bash"]
