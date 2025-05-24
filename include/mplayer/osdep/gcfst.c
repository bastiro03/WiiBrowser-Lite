/*

libgcfst -- a GC disc filesystem devoptab library for the Wii

Copyright (C) 2010 Dimok

Based on the libfst by Joseph Jordan <joe.ftpii@psychlaw.com.au>

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from
the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software in a
product, an acknowledgment in the product documentation would be
appreciated but is not required.

2.Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3.This notice may not be removed or altered from any source distribution.

*/
#include <di/di.h>
#include <errno.h>
#include <ogc/es.h>
#include <ogc/lwp_watchdog.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/dir.h>
#include <sys/iosupport.h>
#include "gcfst.h"
#include "plat_gekko.h"

#define DEVICE_NAME "dvd"

#define FLAG_DIR 1

#define FST_MAXPATHLEN 255
#define DIR_SEPARATOR '/'
#define SECTOR_SIZE 0x800
#define BUFFER_SIZE 0x8000

typedef struct {
    u32 dol_offset;
    u32 fst_offset;
    u32 fst_size;
    u32 fst_size2;
} __attribute__ ((packed)) FST_INFO;

typedef struct DIR_ENTRY_STRUCT {
    char *name;
    u32 offset;
    u32 size;
    u8 flags;
    u32 fileCount;
    struct DIR_ENTRY_STRUCT *children;
} DIR_ENTRY;

typedef struct {
    DIR_ENTRY *entry;
    u32 offset;
    bool inUse;
} FILE_STRUCT;

typedef struct {
    DIR_ENTRY *entry;
    u32 index;
    bool inUse;
} DIR_STATE_STRUCT;

static u8 * read_buffer = NULL;
static u32 cache_start = 0;
static u32 cache_sectors = 0;

static DIR_ENTRY *root = NULL;
static DIR_ENTRY *current = NULL;
static s32 dotab_device = -1;

static bool is_dir(DIR_ENTRY *entry) {
    return entry->flags & FLAG_DIR;
}

static bool invalid_drive_specifier(const char *path) {
    if (strchr(path, ':') == NULL) return false;
    int namelen = strlen(DEVICE_NAME);
    if (!strncmp(DEVICE_NAME, path, namelen) && path[namelen] == ':') return false;
    return true;
}

static DIR_ENTRY *entry_from_path(const char *path) {
    if (invalid_drive_specifier(path)) return NULL;
    if (strchr(path, ':') != NULL) path = strchr(path, ':') + 1;
    DIR_ENTRY *entry;
    bool found = false;
    bool notFound = false;
    const char *pathPosition = path;
    const char *pathEnd = strchr(path, '\0');
    if (pathPosition[0] == DIR_SEPARATOR) {
        entry = root;
        while (pathPosition[0] == DIR_SEPARATOR) pathPosition++;
        if (pathPosition >= pathEnd) found = true;
    } else {
        entry = current;
    }
    if (entry == root && !strcmp(".", pathPosition)) found = true;
    DIR_ENTRY *dir = entry;
    while (!found && !notFound) {
        const char *nextPathPosition = strchr(pathPosition, DIR_SEPARATOR);
        size_t dirnameLength;
        if (nextPathPosition != NULL) dirnameLength = nextPathPosition - pathPosition;
        else dirnameLength = strlen(pathPosition);
        if (dirnameLength >= FST_MAXPATHLEN) return NULL;

        u32 fileIndex = 0;
        while (fileIndex < dir->fileCount && !found && !notFound) {
            entry = &dir->children[fileIndex];
            if (dirnameLength == strnlen(entry->name, FST_MAXPATHLEN - 1) && !strncasecmp(pathPosition, entry->name, dirnameLength)) found = true;
            if (found && !is_dir(entry) && nextPathPosition) found = false;
            if (!found) fileIndex++;
        }

        if (fileIndex >= dir->fileCount) {
            notFound = true;
            found = false;
        } else if (!nextPathPosition || nextPathPosition >= pathEnd) {
            found = true;
        } else if (is_dir(entry)) {
            dir = entry;
            pathPosition = nextPathPosition;
            while (pathPosition[0] == DIR_SEPARATOR) pathPosition++;
            if (pathPosition >= pathEnd) found = true;
            else found = false;
        }
    }

    if (found && !notFound) return entry;
    return NULL;
}

static int _FST_open_r(struct _reent *r, void *fileStruct, const char *path, int flags, int mode) {
    FILE_STRUCT *file = (FILE_STRUCT *)fileStruct;
    DIR_ENTRY *entry = entry_from_path(path);
    if (!entry) {
        r->_errno = ENOENT;
        return -1;
    } else if (is_dir(entry)) {
        r->_errno = EISDIR;
        return -1;
    }

    file->entry = entry;
    file->offset = 0;
    file->inUse = true;

    return (int)file;
}

static int _FST_close_r(struct _reent *r, int fd) {
    FILE_STRUCT *file = (FILE_STRUCT *)fd;
    if (!file->inUse) {
        r->_errno = EBADF;
        return -1;
    }
    file->inUse = false;
    return 0;
}

static int internal_read(void *ptr, u64 offset, size_t len)
{
    u32 sector = offset / SECTOR_SIZE;
    u32 end_sector = (offset + len - 1) / SECTOR_SIZE;
    u32 sectors = MIN(BUFFER_SIZE / SECTOR_SIZE, end_sector - sector + 1);
    u32 sector_offset = offset % SECTOR_SIZE;
    len = MIN(BUFFER_SIZE - sector_offset, len);
    if (cache_sectors && sector >= cache_start && (sector + sectors) <= (cache_start + cache_sectors)) {
        memcpy(ptr, read_buffer + (sector - cache_start) * SECTOR_SIZE + sector_offset, len);
        return len;
    }
    DVDGekkoTick(true);
    if (DI_ReadDVD(read_buffer, BUFFER_SIZE / SECTOR_SIZE, sector)) {
        cache_sectors = 0;
        u32 error;
        if (DI_GetError(&error)) return -1;
        if ((error & 0xFFFFFF) == 0x020401) { // discid has to be read again
            u64 discid;
            DI_ReadDiscID(&discid);
            if (DI_ReadDVD(read_buffer, BUFFER_SIZE / SECTOR_SIZE, sector))
                return -1;
        }
    }
    cache_start = sector;
    cache_sectors = BUFFER_SIZE / SECTOR_SIZE;
    memcpy(ptr, read_buffer + sector_offset, len);
    return len;
}

static int _read(void *ptr, u64 offset, u32 len)
{
    s32 read = 0;
    u32 done = 0;
    u8 * dataptr = (u8 *) ptr;

    while(done < len)
    {
        if((read = internal_read(dataptr+done, offset+done, len-done)) < 0) return read;

        done += read;
    }

    return done;
}

static int _FST_read_r(struct _reent *r, int fd, char *ptr, size_t len) {
    FILE_STRUCT *file = (FILE_STRUCT *)fd;
    if (!file->inUse) {
        r->_errno = EBADF;
        return -1;
    }
    if (file->offset >= file->entry->size) {
        r->_errno = EOVERFLOW;
        return 0;
    }
    if (len + file->offset > file->entry->size) {
        r->_errno = EOVERFLOW;
        len = file->entry->size - file->offset;
    }
    if (len <= 0) {
        return 0;
    }

    len = _read(ptr, file->entry->offset + file->offset, len);
    if (len < 0)
    {
        r->_errno = EIO;
        return -1;
    }

    file->offset += len;
    return len;
}

static off_t _FST_seek_r(struct _reent *r, int fd, off_t pos, int dir) {
    FILE_STRUCT *file = (FILE_STRUCT *)fd;
    if (!file->inUse) {
        r->_errno = EBADF;
        return -1;
    }

    int position;

    switch (dir) {
        case SEEK_SET:
            position = pos;
            break;
        case SEEK_CUR:
            position = file->offset + pos;
            break;
        case SEEK_END:
            position = file->entry->size + pos;
            break;
        default:
            r->_errno = EINVAL;
            return -1;
    }

    if (pos > 0 && position < 0) {
        r->_errno = EOVERFLOW;
        return -1;
    }

    if (position < 0 || position > file->entry->size) {
        r->_errno = EINVAL;
        return -1;
    }

    file->offset = position;

    return position;
}

static void stat_entry(DIR_ENTRY *entry, struct stat *st) {
    st->st_dev = 0x4657;
    st->st_ino = 0;
    st->st_mode = ((is_dir(entry)) ? S_IFDIR : S_IFREG) | (S_IRUSR | S_IRGRP | S_IROTH);
    st->st_nlink = 1;
    st->st_uid = 1;
    st->st_gid = 2;
    st->st_rdev = st->st_dev;
    st->st_size = entry->size;
    st->st_atime = 0;
    st->st_spare1 = 0;
    st->st_mtime = 0;
    st->st_spare2 = 0;
    st->st_ctime = 0;
    st->st_spare3 = 0;
    st->st_blksize = SECTOR_SIZE;
    st->st_blocks = (entry->size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    st->st_spare4[0] = 0;
    st->st_spare4[1] = 0;
}

static int _FST_fstat_r(struct _reent *r, int fd, struct stat *st) {
    FILE_STRUCT *file = (FILE_STRUCT *)fd;
    if (!file->inUse) {
        r->_errno = EBADF;
        return -1;
    }
    stat_entry(file->entry, st);
    return 0;
}

static int _FST_stat_r(struct _reent *r, const char *path, struct stat *st) {
    DIR_ENTRY *entry = entry_from_path(path);
    if (!entry) {
        r->_errno = ENOENT;
        return -1;
    }
    stat_entry(entry, st);
    return 0;
}

static int _FST_chdir_r(struct _reent *r, const char *path) {
    DIR_ENTRY *entry = entry_from_path(path);
    if (!entry) {
        r->_errno = ENOENT;
        return -1;
    } else if (!is_dir(entry)) {
        r->_errno = ENOTDIR;
        return -1;
    }
    return 0;
}

static DIR_ITER *_FST_diropen_r(struct _reent *r, DIR_ITER *dirState, const char *path) {
    DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
    state->entry = entry_from_path(path);
    if (!state->entry) {
        r->_errno = ENOENT;
        return NULL;
    } else if (!is_dir(state->entry)) {
        r->_errno = ENOTDIR;
        return NULL;
    }
    state->index = 0;
    state->inUse = true;
    return dirState;
}

static int _FST_dirreset_r(struct _reent *r, DIR_ITER *dirState) {
    DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
    if (!state->inUse) {
        r->_errno = EBADF;
        return -1;
    }
    state->index = 0;
    return 0;
}

static int _FST_dirnext_r(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st) {
    DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
    if (!state->inUse) {
        r->_errno = EBADF;
        return -1;
    }
    if (state->index >= state->entry->fileCount) {
        r->_errno = ENOENT;
        return -1;
    }
    DIR_ENTRY *entry = &state->entry->children[state->index++];
    strncpy(filename, entry->name, FST_MAXPATHLEN - 1);
    stat_entry(entry, st);
    return 0;
}

static int _FST_dirclose_r(struct _reent *r, DIR_ITER *dirState) {
    DIR_STATE_STRUCT *state = (DIR_STATE_STRUCT *)(dirState->dirStruct);
    if (!state->inUse) {
        r->_errno = EBADF;
        return -1;
    }
    state->inUse = false;
    return 0;
}

static int _FST_statvfs_r(struct _reent *r, const char *name, struct statvfs *buf)
{
    if(!buf)
        return -1;

    memset(buf, 0,  sizeof(struct statvfs));
    return 0;
}


static const devoptab_t dotab_fst = {
    DEVICE_NAME,
    sizeof(FILE_STRUCT),
    _FST_open_r,
    _FST_close_r,
    NULL,
    _FST_read_r,
    _FST_seek_r,
    _FST_fstat_r,
    _FST_stat_r,
    NULL,
    NULL,
    _FST_chdir_r,
    NULL,
    NULL,
    sizeof(DIR_STATE_STRUCT),
    _FST_diropen_r,
    _FST_dirreset_r,
    _FST_dirnext_r,
    _FST_dirclose_r,
    _FST_statvfs_r
};

typedef struct {
    u8 disc_id;
    u8 game_code[2];
    u8 region_code;
    u8 maker_code[2];
    u8 disc_id2;
    u8 disc_version;
    u8 audio_streaming;
    u8 streaming_buffer_size;
    u8 unused[0x12];
    u32 magic;
    u8 title[64];
} __attribute__((packed)) DISC_HEADER;

typedef struct {
    u32 count;
    u32 table_offset;
} __attribute__((packed)) PARTITION_TABLE_ENTRY;

typedef struct {
    u32 offset;
    u32 type;
} __attribute__((packed)) PARTITION_ENTRY;

typedef struct {
    u8 filetype;
    char name_offset[3];
    u32 fileoffset;
    u32 filelen;
} __attribute__((packed)) FST_ENTRY;

static DIR_ENTRY *add_child_entry(DIR_ENTRY *dir, const char *name) {
    DIR_ENTRY *newChildren = realloc(dir->children, (dir->fileCount + 1) * sizeof(DIR_ENTRY));
    if (!newChildren) return NULL;
    bzero(newChildren + dir->fileCount, sizeof(DIR_ENTRY));
    dir->children = newChildren;
    DIR_ENTRY *child = &dir->children[dir->fileCount++];
    child->name = strdup(name);
    if (!child->name) return NULL;
    return child;
}

static s32 read_fst(DIR_ENTRY *entry, FST_ENTRY *fst, char *name_table, s32 index) {
    FST_ENTRY *fst_entry = fst + index;

    if (index > 0) {
        u32 name_offset = *((u32 *)fst_entry) & 0x00ffffff;
        char *name = name_table + name_offset;
        if (strlen(name) >= FST_MAXPATHLEN) return -1;
        entry = add_child_entry(entry, name);
        if (!entry) return -1;
        entry->flags = fst_entry->filetype;
    }

    if (fst_entry->filetype & FLAG_DIR) {
        if(entry != root) add_child_entry(entry, "..")->flags = FLAG_DIR;
        entry->offset = index;
        s32 next;
        for (next = index + 1; next < fst_entry->filelen;) {
            next = read_fst(entry, fst, name_table, next);
            if (next == -1) return -1;
        }
        return fst_entry->filelen;
    } else {
        entry->offset = fst_entry->fileoffset;
        entry->size = fst_entry->filelen;
        return index + 1;
    }
}

static bool read_partition(FST_INFO *fst_info) {
    bool result = false;

    u32 fst_size = fst_info->fst_size;
    u32 fst_offset = fst_info->fst_offset;
    u8 *fst_buffer = malloc(fst_size);
    if (!fst_buffer) goto end;

    if(_read(fst_buffer, fst_offset, fst_size) < 0) goto end;

    FST_ENTRY *fst = (FST_ENTRY *)fst_buffer;
    u32 name_table_offset = fst->filelen * sizeof(FST_ENTRY);
    char *name_table = (char *)fst_buffer + name_table_offset;

    result = read_fst(root, fst, name_table, 0) != -1;

    end:
    if (fst_buffer) free(fst_buffer);
    return result;
}

static bool add_header_entry(DIR_ENTRY *parent) {
    DIR_ENTRY *entry = add_child_entry(parent, "header.bin");
    if (!entry) return false;
    entry->size = 0x400;
    return true;
}

static bool read_appldr_size(DIR_ENTRY *appldr)
{
    u32 ints[2];
    if (_read(&ints[0], appldr->offset + 0x14, 8) < 0) return false;
    u32 size = ints[0] + ints[1];
    if (size) size += 32;
    appldr->size = size;
    return true;
}

static bool add_appldr_entry(DIR_ENTRY *parent) {
    DIR_ENTRY *entry = add_child_entry(parent, "appldr.bin");
    if (!entry) return false;
    entry->offset = 0x2440;
    return read_appldr_size(entry);
}

static bool read_dol_size(DIR_ENTRY *dol)
{
    u8 temp_buffer[0x100];
    if (_read(temp_buffer, dol->offset, 0x100) < 0) return false;
    u32 max = 0;
    u32 i;
    for (i = 0; i < 7; i++) {
        u32 offset = *(u32 *)(temp_buffer + (i * 4));
        u32 size = *(u32 *)(temp_buffer + (i * 4) + 0x90);
        if ((offset + size) > max) max = offset + size;
    }
    for (i = 0; i < 11; i++) {
        u32 offset = *(u32 *)(temp_buffer + (i * 4) + 0x1c);
        u32 size = *(u32 *)(temp_buffer + (i * 4) + 0xac);
        if ((offset + size) > max) max = offset + size;
    }
    dol->size = max;
    return true;
}

static bool add_dol_entry(DIR_ENTRY *parent, FST_INFO * disc_fst) {
    DIR_ENTRY *entry = add_child_entry(parent, "main.dol");
    if (!entry) return false;
    entry->offset = disc_fst->dol_offset;
    return read_dol_size(entry);
}

static bool add_fst_entry(DIR_ENTRY *parent, FST_INFO * disc_fst) {
    DIR_ENTRY *entry = add_child_entry(parent, "fst.bin");
    if (!entry) return false;
    entry->offset = disc_fst->fst_offset;
    entry->size = disc_fst->fst_size;
    return true;
}

static bool read_disc()
{
    if (DI_ReadDVD(read_buffer, 1, 0)) return false;
    DISC_HEADER *header = (DISC_HEADER *)read_buffer;
    if (header->magic != 0xc2339f3d) return false;

    if(root) free(root);
    if (!(root = malloc(sizeof(DIR_ENTRY)))) return false;
    bzero(root, sizeof(DIR_ENTRY));
    root->flags = FLAG_DIR;
    current = root;

    FST_INFO disc_fst;
    if (_read(&disc_fst, 0x420, sizeof(FST_INFO)) < 0) goto error;
    if (!add_header_entry(root)) goto error;
    if (!add_appldr_entry(root)) goto error;
    if (disc_fst.dol_offset) {
        if (!add_dol_entry(root, &disc_fst)) goto error;
    }
    if (disc_fst.fst_offset && disc_fst.fst_size) {
        if (!add_fst_entry(root, &disc_fst)) goto error;
        if (!read_partition(&disc_fst)) goto error;
    }

    return true;
    error:
    free(root);
    root = NULL;
    return false;
}

static void cleanup_recursive(DIR_ENTRY *entry) {
    u32 i;
    for (i = 0; i < entry->fileCount; i++) cleanup_recursive(&entry->children[i]);
    if (entry->children) free(entry->children);
    if (entry->name) free(entry->name);
}

bool GCFST_Mount()
{
    GCFST_Unmount();
    if((read_buffer = memalign(32, BUFFER_SIZE)) == NULL) return false;
    bool success = read_disc() && (dotab_device = AddDevice(&dotab_fst)) >= 0;
    if (!success) GCFST_Unmount();
    return success;
}

bool GCFST_Unmount()
{
    if (root)
    {
        cleanup_recursive(root);
        free(root);
        root = NULL;
    }
    if(read_buffer)
    {
        free(read_buffer);
        read_buffer = NULL;
    }
    current = root;
    cache_start = 0;
    cache_sectors = 0;
    if (dotab_device >= 0)
    {
        dotab_device = -1;
        return !RemoveDevice(DEVICE_NAME ":");
    }
    return true;
}
