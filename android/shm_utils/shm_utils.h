/*
 * Shared memory utility functions
 *
 * Copyright (C) 2018 Zebediah Figura
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __WINE_SERVER_SHM_UTILS_H
#define __WINE_SERVER_SHM_UTILS_H

#ifdef __ANDROID__

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>

#define MAX_SHM_SEGMENTS 512
#define MASTER_SHM_FILE "wine_shm_master"
#define PAGE_SIZE 4096

typedef struct {
    char name[256];
    off_t offset;
    size_t size;
    int in_use;
} ShmEntry;

typedef struct {
    off_t global_cursor;
    ShmEntry entries[MAX_SHM_SEGMENTS];
} ShmRegistry;

/* Process-local globals */
static ShmRegistry *registry = NULL;
static int master_fd = -1;
static int fd_to_index[1024]; // Maps local_fd -> registry_index

/* 1. INITIALIZE SHM REGISTRY */
static inline void init_shm_registry(void) {
    if (registry != NULL) return;

    char *tmpdir = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
    char fname[512];
    snprintf(fname, sizeof(fname), "%s/%s", tmpdir, MASTER_SHM_FILE);

    master_fd = open(fname, O_RDWR | O_CREAT, 0666);
    if (master_fd == -1) return;

    struct stat st;
    fstat(master_fd, &st);

    /* If the file is smaller than our header, expand it */
    if (st.st_size < sizeof(ShmRegistry)) {
        ftruncate(master_fd, sizeof(ShmRegistry));
    }

    registry = (ShmRegistry *)mmap(NULL, sizeof(ShmRegistry),
                                   PROT_READ | PROT_WRITE, MAP_SHARED,
                                   master_fd, 0);

    if (registry == MAP_FAILED) {
        registry = NULL;
        return;
    }

    /* Initialize metadata if we are the first process to create the file */
    if (st.st_size < sizeof(ShmRegistry)) {
        registry->global_cursor = PAGE_SIZE; // Data starts after the first page
        for (int i = 0; i < MAX_SHM_SEGMENTS; i++) registry->entries[i].in_use = 0;
    }

    /* Initialize the process-local FD map */
    for (int i = 0; i < 1024; i++) fd_to_index[i] = -1;
}

/* 2. OPEN SHM (VIRTUAL) */
static inline int shm_open_virtual(const char *name, int oflag, mode_t mode) {
    init_shm_registry();
    if (!registry) return -1;

    flock(master_fd, LOCK_EX);

    int idx = -1;
    /* Check for existing segment */
    for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
        if (registry->entries[i].in_use && strcmp(registry->entries[i].name, name) == 0) {
            idx = i;
            break;
        }
    }

    /* Create new segment if not found */
    if (idx == -1) {
        for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
            if (!registry->entries[i].in_use) {
                snprintf(registry->entries[i].name, sizeof(registry->entries[i].name), "%s", name);
                registry->entries[i].in_use = 1;
                registry->entries[i].offset = registry->global_cursor;
                registry->entries[i].size = 0;
                idx = i;
                break;
            }
        }
    }

    flock(master_fd, LOCK_UN);

    if (idx != -1) {
        /* dup() gives the caller a real file descriptor that the OS recognizes */
        int new_fd = dup(master_fd);
        if (new_fd >= 0 && new_fd < 1024) fd_to_index[new_fd] = idx;
        return new_fd;
    }
    return -1;
}

/* 3. FTRUNCATE SHM (VIRTUAL) */
static inline int ftruncate_virtual(int fd, off_t length) {
    if (fd < 0 || fd >= 1024 || fd_to_index[fd] == -1) return -1;
    int idx = fd_to_index[fd];

    flock(master_fd, LOCK_EX);
    registry->entries[idx].size = length;
    off_t end_pos = registry->entries[idx].offset + length;

    /* Physically grow the master file if this segment extends beyond current file size */
    if (end_pos > registry->global_cursor) {
        ftruncate(master_fd, end_pos);
        /* Align next segment to the next page boundary */
        registry->global_cursor = (end_pos + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    }
    flock(master_fd, LOCK_UN);
    return 0;
}

/* 4. MMAP SHM (VIRTUAL) */
static inline void* mmap_virtual(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (fd < 0 || fd >= 1024 || fd_to_index[fd] == -1) return MAP_FAILED;
    int idx = fd_to_index[fd];

    /* Translate local segment offset to the global master file offset */
    off_t true_offset = registry->entries[idx].offset + offset;
    return mmap(addr, length, prot, flags, master_fd, true_offset);
}

/* 5. UNLINK SHM (VIRTUAL) */
static inline int shm_unlink_virtual(const char *name) {
    init_shm_registry();
    if (!registry) return -1;

    flock(master_fd, LOCK_EX);
    for (int i = 0; i < MAX_SHM_SEGMENTS; i++) {
        if (registry->entries[i].in_use && strcmp(registry->entries[i].name, name) == 0) {
            registry->entries[i].in_use = 0;
            break;
        }
    }
    flock(master_fd, LOCK_UN);
    return 0;
}

static inline int shm_open(const char *name, int oflag, mode_t mode) {
    char *tmpdir;
    char *fname;

    tmpdir = getenv("TMPDIR");

    if (!tmpdir) {
        tmpdir = "/tmp";
    }

    asprintf(&fname, "%s/%s", tmpdir, name);
    return open(fname, oflag, mode);
}

static inline int shm_unlink(const char *name) {
    char *tmpdir;
    char *fname;

    tmpdir = getenv("TMPDIR");

    if (!tmpdir) {
        tmpdir = "/tmp";
    }

    asprintf(&fname, "%s/%s", tmpdir, name);
    return unlink(fname);
}

#endif /* __ANDROID__ */

#endif /* __WINE_SERVER_SHM_UTILS_H */
