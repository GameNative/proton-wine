/*
 * Kernel /dev/ntsync ioctl ABI, for runtime kernel-vs-userspace detection.
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
 * Mirrors the ioctl request numbers from the Linux uapi header
 * (include/uapi/linux/ntsync.h); those numbers are frozen kernel ABI.
 * The struct layouts come from ntsync_user.h, which mirrors the same uapi
 * structs, so this header only adds the ioctl numbers. uint32_t stands in
 * for the uapi __u32 (identical size, hence identical ioctl numbers).
 *
 * Used on Android, where /dev/ntsync may or may not exist and be usable:
 * wineserver/ntdll compile in both the kernel-ioctl and the userspace
 * implementation and pick one at runtime.
 *
 * SPDX-License-Identifier: LGPL-3.0-only
 */
#ifndef NTSYNC_KERNEL_ABI_H
#define NTSYNC_KERNEL_ABI_H

#include <sys/ioctl.h>

#include "ntsync_user.h"

#define NTSYNC_IOC_CREATE_SEM       _IOW ('N', 0x80, struct ntsync_sem_args)
#define NTSYNC_IOC_SEM_RELEASE      _IOWR('N', 0x81, uint32_t)
#define NTSYNC_IOC_WAIT_ANY         _IOWR('N', 0x82, struct ntsync_wait_args)
#define NTSYNC_IOC_WAIT_ALL         _IOWR('N', 0x83, struct ntsync_wait_args)
#define NTSYNC_IOC_CREATE_MUTEX     _IOW ('N', 0x84, struct ntsync_mutex_args)
#define NTSYNC_IOC_MUTEX_UNLOCK     _IOWR('N', 0x85, struct ntsync_mutex_args)
#define NTSYNC_IOC_MUTEX_KILL       _IOW ('N', 0x86, uint32_t)
#define NTSYNC_IOC_CREATE_EVENT     _IOW ('N', 0x87, struct ntsync_event_args)
#define NTSYNC_IOC_EVENT_SET        _IOR ('N', 0x88, uint32_t)
#define NTSYNC_IOC_EVENT_RESET      _IOR ('N', 0x89, uint32_t)
#define NTSYNC_IOC_EVENT_PULSE      _IOR ('N', 0x8a, uint32_t)
#define NTSYNC_IOC_SEM_READ         _IOR ('N', 0x8b, struct ntsync_sem_args)
#define NTSYNC_IOC_MUTEX_READ       _IOR ('N', 0x8c, struct ntsync_mutex_args)
#define NTSYNC_IOC_EVENT_READ       _IOR ('N', 0x8d, struct ntsync_event_args)

#endif /* NTSYNC_KERNEL_ABI_H */
