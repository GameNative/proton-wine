# Proton Wine Build Workflow

This GitHub Actions workflow automatically builds Proton Wine for both x86_64 and aarch64 (ARM64EC) architectures.

## Workflow Overview

The workflow consists of two main jobs:

### 1. Build Job (Matrix Strategy)
Builds Proton Wine for both architectures in parallel:
- **x86_64**: Intel/AMD 64-bit architecture
- **aarch64**: ARM64EC architecture

### 2. Release Job
Combines all build artifacts and creates a GitHub release (only on push to main/master branch).

## Build Process

The build follows these steps for each architecture:

1. **Environment Setup**
   - Frees up disk space by removing unnecessary tools
   - Installs required build dependencies
   - Downloads architecture-specific termuxfs from [GameNative/termux-on-gha](https://github.com/GameNative/termux-on-gha/releases/tag/build-20260218)

2. **Toolchain Setup** (with caching)
   - Android NDK r27d (version 27.3.13750724)
   - LLVM MinGW toolchain (20250920)

3. **Wine Tools Build** (with caching)
   - Builds native wine-tools using `build-step0.sh`
   - Cached based on configure files hash

4. **Architecture-Specific Build**
   - Builds sysvshm library (aarch64 only)
   - Configures build with appropriate flags
   - Compiles Proton Wine
   - Installs to output directory

5. **Artifact Packaging**
   - Downloads `prefixPack.txz` from [GameNative/bionic-prefix-files](https://github.com/GameNative/bionic-prefix-files)
   - Generates `profile.json` with build metadata
   - Creates **WCP (Winlator Compatible Package)** in txz format containing:
     - `bin/` - Wine binaries
     - `lib/` - Wine libraries
     - `share/` - Wine data files
     - `prefixPack.txz` - Bionic prefix files
     - `profile.json` - Build metadata

## Triggers

The workflow runs on:
- **Push** to `main`, `master`, or `proton_10.0` branch
- **Pull requests** to `main`, `master`, or `proton_10.0` branch
- **Manual trigger** via workflow_dispatch

## Caching Strategy

To speed up builds, the workflow caches:
- Android NDK (key: `android-ndk-r27d`)
- LLVM MinGW toolchain (key: `bylaws-llvm-mingw-20250920`)
- Wine tools (key: `wine-tools-<hash of configure files>`)

## Artifacts

Build artifacts are:
- Uploaded to GitHub Actions artifacts (30-day retention)
- Included in GitHub releases (for pushes to main/master/proton_10.0)

### Artifact Names

**WCP Files (Winlator Compatible Package)**
- `proton-10.0-4-x86_64.wcp` - x86_64 WCP (xz compressed, txz format)
- `proton-10.0-4-arm64ec.wcp` - arm64ec WCP (xz compressed, txz format)

## Build Scripts Used

The workflow utilizes the following scripts from `build-scripts/`:
- `build-step0.sh` - Builds wine-tools
- `build-step-x86_64.sh` - x86_64 build configuration and execution
- `build-step-arm64ec.sh` - aarch64/ARM64EC build configuration and execution

## Requirements

### External Dependencies
- **Termuxfs**: Pre-built termux filesystem from GameNative/termux-on-gha
- **Android NDK**: r27d from Google
- **LLVM MinGW**: 20250920 release from bylaws/llvm-mingw

### Build Dependencies
- build-essential
- git, wget, curl, unzip
- flex, bison, gettext
- autoconf, automake, libtool
- pkg-config
- mingw-w64
- gcc-multilib, g++-multilib

## Customization

### Changing Architectures
Modify the matrix strategy in the workflow:
```yaml
strategy:
  matrix:
    arch: [x86_64, aarch64]  # Add or remove architectures
```

### Updating Termuxfs Version
Change the download URL in the "Download and extract termuxfs" step:
```yaml
wget https://github.com/GameNative/termux-on-gha/releases/download/<TAG>/termuxfs-${{ matrix.arch }}.tar.gz
```

### Modifying Build Configuration
Edit the corresponding build script in `build-scripts/`:
- `build-step-x86_64.sh` for x86_64 configuration
- `build-step-arm64ec.sh` for aarch64 configuration

## Troubleshooting

### Build Failures
1. Check the build logs in GitHub Actions
2. Verify termuxfs download is successful
3. Ensure all patches apply correctly
4. Check disk space availability

### Cache Issues
If caching causes problems, you can:
1. Manually clear caches in GitHub repository settings
2. Update cache keys in the workflow file

### Release Issues
Ensure the `GITHUB_TOKEN` has appropriate permissions:
- Go to repository Settings → Actions → General
- Set "Workflow permissions" to "Read and write permissions"
