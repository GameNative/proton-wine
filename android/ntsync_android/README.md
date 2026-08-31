# ntsync_android integration

Userspace replacement of the Linux `ntsync` kernel driver for Android
(devices without a usable `/dev/ntsync`). The library source lives in a
separate project:

- Local: `../ntsync-android` (i.e. `$PROJECT_ROOT/../ntsync-android`)
- Upstream: https://github.com/joshuatam/ntsync-android

`ntsync_user.h` in this directory is a vendored copy of
`include/ntsync_user.h` from that project; the Wine patches under
`android/patches/common/*ntsync*` include it via a relative path, like
`android/shm_utils`.

`ntsync_kernel_abi.h` carries the kernel `/dev/ntsync` ioctl request
numbers (frozen uapi, mirroring `include/uapi/linux/ntsync.h`) on top of
the same struct definitions from `ntsync_user.h`. It lets wineserver/ntdll
compile in **both** the kernel-ioctl and the userspace implementation and
pick one at runtime.

## Building the library

The build scripts (`build-scripts/build-step-arm64ec.sh`,
`build-scripts/build-step-x86_64.sh`) provide a `--build-ntsync-android`
step which:

1. Uses `$NTSYNC_ANDROID_DIR` if set, else `../ntsync-android` if present,
   else clones the GitHub repo into `android/ntsync-android/`.
2. Runs its `build-scripts/build-android.sh --build` (Rust/cargo + NDK,
   always 16KB-aligned).
3. Copies the matching `libntsync_android.a` for the ABI being built into
   `$deps/lib/` (static archive; linked directly into ntdll/wineserver).

## Runtime backend selection

wineserver probes for in-process sync support once at startup
(`get_inproc_device_fd()` in `server/inproc_sync.c`):

1. `PROTON_NO_NTSYNC=1` disables ntsync entirely (both kernel and
   userspace); wineserver falls back to server-side synchronization.
2. Otherwise it opens `/dev/ntsync` and **probes** it with a real
   `NTSYNC_IOC_CREATE_EVENT` ioctl (the node can exist but be unusable,
   e.g. SELinux policy or seccomp). If the probe succeeds, the kernel
   driver is used, exactly like upstream Proton.
3. If the device is missing or the probe fails, it tries
   `ntsync_init()`; on success all processes attach to the shared-memory
   region and use the userspace implementation
   (`ntsync: no usable /dev/ntsync, using userspace ntsync.`).
4. If that also fails, wineserver falls back to server-side
   synchronization.

wineserver tells each client which backend is in use through the
`init_first_thread` reply: a passed device fd (kernel), the
`NTSYNC_ANDROID_USED_BY_SERVER` sentinel (userspace), or nothing. ntdll
compiles in both implementations and dispatches per call at runtime;
there is no per-object mixing — the whole server session uses one backend.

In userspace mode the integer object handles are passed from wineserver to
clients in the `fsync_shm_idx` field of the `get_inproc_sync_fd` /
`get_inproc_alert_fd` replies instead of `SCM_RIGHTS` fd passing. All Wine
processes attach to the same shared-memory region
(`$TMPDIR/ntsync_userspace.shm`); make sure `TMPDIR` is exported (Termux
does this by default).
