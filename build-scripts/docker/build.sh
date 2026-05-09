#!/bin/bash

if [[ "$CONTAINER" != "1" ]];
then
    DOCKER=podman
    SRC_PATH="$HOME/programming/proton-wine"
    OUTPUT_PATH="$HOME/proton_builds"
    RELEASE=10.0-2
fi
#This is overengineered and hacky.
#It's basically a 1-1 copy of the github workflow
#Could probably be made a lot simpler, but here we are

make_package() {
    # $1 is arch
    # $2 is platform
    # $3 is arch_name
    BIN_DIR="$HOME/compiled-files-$1"
    echo "moving to $BIN_DIR ..."
    wget https://github.com/GameNative/bionic-prefix-files/raw/main/prefixPack-$3.txz -O $BIN_DIR/prefixPack.txz
    if [[ "$2" == "GN" ]];
    then
		cat > $BIN_DIR/profile.json <<-EPROFILE
		{
		  "type": "Proton",
		  "versionName": "$RELEASE-$3",
		  "versionCode": 1,
		  "description": "Proton $RELEASE $3 - Windows compatibility layer with improved gaming support",
		  "files": [],
		  "wine": {
		    "binPath": "bin",
		    "libPath": "lib",
		    "prefixPack": "prefixPack.txz"
		  }
		}
		EPROFILE
        cd $BIN_DIR
        echo "Building Release..."
        tar cJf proton-$RELEASE-$3.wcp bin lib share prefixPack.txz profile.json
        mv proton-$RELEASE-$3.wcp "$OUTPUT_PATH/"
    fi
    if [[ "$2" == "WIN" ]];
    then
		cat > $BIN_DIR/profile-wine.json <<-EPROFILE
		{
		  "type": "Wine",
		  "versionName": "$RELEASE-$3",
		  "versionCode": 0,
		  "description": "Proton $RELEASE $3 - Windows compatibility layer with improved gaming support",
		  "files": [],
		  "wine": {
		    "binPath": "bin",
		    "libPath": "lib",
		    "prefixPack": "prefixPack.txz"
		  }
		}
		EPROFILE
        cd $BIN_DIR
        echo "Building Release..."
        tar cJf proton-wine-$RELEASE-$3.wcp.xz bin lib share prefixPack.txz profile-wine.json
        mv proton-wine-$RELEASE-$3.wcp.xz $OUTPUT_PATH/
    fi
}

if [[ "$CONTAINER" != "1" ]];
then
    
    mkdir -p "$OUTPUT_PATH"
    ARCH_ARM=yes
    ARCH_x86=no
    PLATFORM_GN=yes
    PLATFORM_WIN=no
    
    for arg in "$@"
    do
        if [ "$arg" == "--arm64" ];
        then
           ARCH_ARM=yes
           arg_arm=yes
        fi
        if [ "$arg" == "--x86_64" ];
        then
           ARCH_x86=yes
           if [ "$arg_arm" != "yes" ];
           then
               ARCH_ARM=no
           fi
        fi
        if [ "$arg" == "--game_native" ];
        then
           PLATFORM_GN=yes
           arg_gn=yes
        fi
        if [ "$arg" == "--winlator" ];
        then
           PLATFORM_WIN=yes
           if [ "$arg_gn" != "yes" ];
           then
               PLATFORM_GN=no
           fi
        fi
    done
    echo "arm64     : $ARCH_ARM"
    echo "x86_64    : $ARCH_x86"
    echo "GameNative: $PLATFORM_GN"
    echo "Winlator  : $PLATFORM_WIN"
    $DOCKER run --rm \
           -v $OUTPUT_PATH:$OUTPUT_PATH \
           -v $SRC_PATH:$SRC_PATH \
           -w $OUTPUT_PATH \
           -e SRC_PATH="$SRC_PATH" -e OUTPUT_PATH="$OUTPUT_PATH" \
           -e PLATFORM_GN="$PLATFORM_GN" -e PLATFORM_WIN="$PLATFORM_WIN" \
           -e ARCH_ARM="$ARCH_ARM" -e ARCH_x86="$ARCH_x86" \
           -e RELEASE="$RELEASE" \
           -e CONTAINER="1" \
           localhost/proton-wine:latest \
           bash $SRC_PATH/build-scripts/docker/build.sh

else
    
    if [[ "$ARCH_ARM" == "yes" ]];
    then
        echo "moving to $SRC_PATH ..."
        cd $SRC_PATH
        bash autogen.sh
        bash build-scripts/build-step0.sh
        rm -rf "$HOME/compiled-files*"
        bash build-scripts/build-step-arm64ec.sh --build-sysvshm
        bash build-scripts/build-step-arm64ec.sh --configure
        bash build-scripts/build-step-arm64ec.sh --build
        bash build-scripts/build-step-arm64ec.sh --install
    
        if [[ "$PLATFORM_GN" == "yes" ]];
        then
            make_package aarch64 GN arm64ec
        fi
    
        if [[ "$PLATFORM_WIN" == "yes" ]];
        then
            make_package aarch64 WIN arm64ec
        fi
    fi
    if [[ "$ARCH_x86" == "yes" ]];
    then
        echo "moving to $SRC_PATH ..."
        cd $SRC_PATH
        bash autogen.sh
        bash build-scripts/build-step0.sh
        rm -rf "$HOME/compiled-files*"
        bash build-scripts/build-step-x86_64.sh --build-sysvshm
        bash build-scripts/build-step-x86_64.sh --configure
        bash build-scripts/build-step-x86_64.sh --build
        bash build-scripts/build-step-x86_64.sh --install
    
        if [[ "$PLATFORM_GN" == "yes" ]];
        then
            make_package x86_64 GN x86_64
        fi
    
        if [[ "$PLATFORM_WIN" == "yes" ]];
        then
            make_package x86_64 WIN x86_64
        fi
    fi
fi

