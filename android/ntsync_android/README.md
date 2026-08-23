# ntsync_android integration

Userspace replacement of the Linux `ntsync` kernel driver for Android
(no `/dev/ntsync` exists there). The library source lives in a separate
project:

- Local: `../ntsync-android` (i.e. `$PROJECT_ROOT/../ntsync-android`)
- Upstream: https://github.com/joshuatam/ntsync-android

`ntsync_user.h` in this directory is a vendored copy of
`include/ntsync_user.h` from that project; the Wine patches under
`android/patches/common/*ntsync*` include it via a relative path, like
`android/shm_utils`.

## Building the library

The build scripts (`build-scripts/build-step-arm64ec.sh`,
`build-scripts/build-step-x86_64.sh`) provide a `--build-ntsync-android`
step which:

1. Uses `$NTSYNC_ANDROID_DIR` if set, else `../ntsync-android` if present,
   else clones the GitHub repo into `android/ntsync-android/`.
2. Runs its `build-scripts/build-android.sh --build` (Rust/cargo + NDK,
   always 16KB-aligned).
3. Copies the matching `libntsync_android.so` for the ABI being built into
   `$deps/lib/` and the header into `$deps/include/`.

The resulting `libntsync_android.so` must be shipped alongside Wine
(on `LD_LIBRARY_PATH` / in the Termux prefix libdir, same as
`libandroid-sysvshm.so`).

## Runtime

All Wine processes attach to the same shared-memory region
(`$TMPDIR/ntsync_userspace.shm`); make sure `TMPDIR` is exported (Termux
does this by default). The integer object handles are passed from
wineserver to clients in the `fsync_shm_idx` field of the
`get_inproc_sync_fd` / `get_inproc_alert_fd` replies instead of
`SCM_RIGHTS` fd passing.
