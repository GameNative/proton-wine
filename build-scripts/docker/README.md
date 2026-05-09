# Instructions for use
1) run `podman build -f ProtonWineBuildImage.Dockerfile -t proton-wine:latest` or `docker build -f ProtonWineBuildImage.Dockerfile -t proton-wine:latest`. This will create the image that holds the android ndk, mingw llvm, and the necessary build tools in an ubuntu 24.04 image
2) Update `build.sh` to point to your source folder and output folder and also whether you are using Docker or Podman
3) run `build.sh` for the default build

The output will be a `*.wcp` file that you can then import into GameNative.

## Build Configuration
    The default options are `--arm64 --game_native`.
    If any of the below are present, only those will be built.
    
### Arch
    --arm64          Build the `arm64` version
    --x86_64         Build the `x86_64` version
### Version
    --game_native    Package the build for GameNative
    --winlator       Package the build for Winlator for CMOD & Ludashi
