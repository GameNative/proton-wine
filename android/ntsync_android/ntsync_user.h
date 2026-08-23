/*
 * Userspace ntsync library for Android - C API.
 *
 * Copyright (C) 2026 Joshua Tam <297250+joshuatam@users.noreply.github.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
 * Mirrors the Linux /dev/ntsync ioctl interface (include/uapi/linux/ntsync.h)
 * with u32 handles in place of kernel fds. All functions return 0 on success
 * or a negative errno, exactly like the kernel ioctls.
 *
 * Objects live in a file-backed shared mapping and are cross-process: any
 * process that opens the same region path sees the same handles. Waits use
 * futexes on the shared pages.
 *
 * Not implemented: alertable waits (ntsync_wait_args.alert must be 0).
 * Divergence from the kernel: closing an object other threads are waiting on
 * fails those waits with -EINVAL; objects leaked by a crashed process must
 * be reclaimed with ntsync_sweep_dead().
 *
 * SPDX-License-Identifier: LGPL-3.0-only
 */
#ifndef NTSYNC_USER_H
#define NTSYNC_USER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ntsync_sem_args {
    uint32_t count;
    uint32_t max;
};

struct ntsync_mutex_args {
    uint32_t owner;
    uint32_t count;
};

struct ntsync_event_args {
    uint32_t manual;
    uint32_t signaled;
};

#define NTSYNC_WAIT_REALTIME 0x1

struct ntsync_wait_args {
    /* Absolute timeout in ns; CLOCK_MONOTONIC, or CLOCK_REALTIME if
     * NTSYNC_WAIT_REALTIME is set. UINT64_MAX = infinite. */
    uint64_t timeout;
    /* Pointer to an array of `count` uint32_t handles. */
    uint64_t objs;
    uint32_t count;
    /* Out: index of the object that satisfied the wait. */
    uint32_t index;
    uint32_t flags;
    /* In: owner tid used to acquire mutexes. */
    uint32_t owner;
    /* Unsupported; must be 0. */
    uint32_t alert;
    uint32_t pad;
};

#define NTSYNC_MAX_WAIT_COUNT 64

/* Wine integration: wineserver reports userspace ntsync to clients by
 * putting this sentinel in the inproc_device field of the init_first_thread
 * reply, and passes object handles in the fsync_shm_idx reply field instead
 * of SCM_RIGHTS fd passing. */
#define NTSYNC_ANDROID_USED_BY_SERVER 0x7eadfe01

/* Initialize the shared region. `path` may be NULL to use
 * $TMPDIR/ntsync_userspace.shm (the caller must export TMPDIR).
 * Idempotent; all other functions auto-initialize on first use. */
int32_t ntsync_init(const char *path);

/* Free all objects whose creator process no longer exists. Userspace has no
 * fd-close-on-death hook, so a launcher/server should call this after a
 * process exits. Returns the number of freed objects or a negative errno. */
int32_t ntsync_sweep_dead(void);

/* Create objects. Return 0 and store the handle, or a negative errno. */
int32_t ntsync_create_sem(uint32_t *out_handle, const struct ntsync_sem_args *args);
int32_t ntsync_create_mutex(uint32_t *out_handle, const struct ntsync_mutex_args *args);
int32_t ntsync_create_event(uint32_t *out_handle, const struct ntsync_event_args *args);

/* Destroy an object. Returns -EINVAL for a bad handle. */
int32_t ntsync_close(uint32_t handle);

/* Semaphores. On success, sem_release overwrites *count with the previous
 * count; returns -EOVERFLOW (state unchanged) if count would exceed max. */
int32_t ntsync_sem_release(uint32_t handle, uint32_t *count);
int32_t ntsync_sem_read(uint32_t handle, struct ntsync_sem_args *args);

/* Mutexes. args->owner is input; on success args->count is overwritten with
 * the previous recursion count. Returns -EPERM if not the owner.
 * mutex_read returns -EOWNERDEAD if the mutex is abandoned. */
int32_t ntsync_mutex_unlock(uint32_t handle, struct ntsync_mutex_args *args);
int32_t ntsync_mutex_kill(uint32_t handle, uint32_t owner);
int32_t ntsync_mutex_read(uint32_t handle, struct ntsync_mutex_args *args);

/* Events. On success, set/reset/pulse store the previous signaled state in
 * *prev (if non-NULL), like the kernel ioctls. */
int32_t ntsync_event_set(uint32_t handle, uint32_t *prev);
int32_t ntsync_event_reset(uint32_t handle, uint32_t *prev);
int32_t ntsync_event_pulse(uint32_t handle, uint32_t *prev);
int32_t ntsync_event_read(uint32_t handle, struct ntsync_event_args *args);

/* Waits. Return 0 and set args->index on success, -EOWNERDEAD (and set
 * args->index) when an abandoned mutex was acquired, -ETIMEDOUT on timeout,
 * -EINVAL on bad arguments. */
int32_t ntsync_wait_any(struct ntsync_wait_args *args);
int32_t ntsync_wait_all(struct ntsync_wait_args *args);

#ifdef __cplusplus
}
#endif

#endif /* NTSYNC_USER_H */
