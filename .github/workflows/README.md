# Proton Wine Build Workflow

`build-proton.yml` builds Proton Wine 11.0-2 for both x86_64 and aarch64 (ARM64EC)
architectures. Single SDK 28 target with 16KB page size support (works on
Android 9+ and Android 15+); there are no longer separate SDK 28/35 workflows.

## Jobs

### 1. Build (matrix: x86_64, aarch64)
1. Environment setup, termuxfs download
2. Toolchain caching: Android NDK r27d, LLVM MinGW 20250920
3. ntsync-android (userspace ntsync) checked out as a sibling repo and built via
   `--build-ntsync-android`
4. ccache install + per-arch cache for fast rebuilds
5. wine-tools (cached), sysvshm, configure, build, install
6. Install pass strips binaries with llvm-strip (tree ~2GB -> ~730MB)
7. Packaging: Proton `.wcp` (zstd, fast in-app install) + Wine `.wcp.xz` (CMOD/Ludashi)

### 2. Release
Runs on push to main/master/proton_11.0-2 or manual dispatch; collects the four
`.wcp` artifacts into a date-tagged release (`build-p11-YYYYMMDD`).
