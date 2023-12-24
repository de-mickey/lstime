// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lstime_private.h"

// #define USE_STAT_AND_LSTAT 1  // uncomment if statx(2) not available

#if defined(AT_STATX_SYNC_TYPE) && ! defined(USE_STAT_AND_LSTAT)

static timespec timespec_from_statx_timestamp(struct statx_timestamp tsx) {
  timespec ts;
  ts.tv_sec = tsx.tv_sec;
  ts.tv_nsec = tsx.tv_nsec;
  return ts;
}

int lstime_stat_path(lstime_info *info, int stat_flags) {
    static int stat_dirfd = AT_FDCWD;
    static unsigned int stat_mask =
        STATX_ATIME | STATX_BTIME | STATX_CTIME | STATX_MTIME;
    struct statx stxbuf;

    int ret = statx(stat_dirfd, info->path, stat_flags, stat_mask, &stxbuf);
    if (ret < 0) {
        return ret;
    }

    if (stxbuf.stx_mask & STATX_MTIME) {
        info->mtime = timespec_from_statx_timestamp(stxbuf.stx_mtime);
    } else {
        SET_TIMESPEC_EMPTY(&info->mtime);
    }

    if (stxbuf.stx_mask & STATX_ATIME) {
        info->atime = timespec_from_statx_timestamp(stxbuf.stx_atime);
    } else {
        SET_TIMESPEC_EMPTY(&info->atime);
    }

    if (stxbuf.stx_mask & STATX_CTIME) {
        info->ctime = timespec_from_statx_timestamp(stxbuf.stx_ctime);
    } else {
        SET_TIMESPEC_EMPTY(&info->ctime);
    }

    if (stxbuf.stx_mask & STATX_BTIME) {
        info->btime = timespec_from_statx_timestamp(stxbuf.stx_btime);
    } else {
        SET_TIMESPEC_EMPTY(&info->btime);
    }

    return 0;
}

#else

int lstime_stat_path(lstime_info *info int stat_flags) {
    struct stat statbuf;

    if (stat_flags & AT_SYMLINK_NOFOLLOW) {
        int ret = lstat(info->path, &statbuf);
        if (ret < 0) {
            return ret;
        }
    } else {
        int ret = lstat(info->path, &statbuf);
        if (ret < 0) {
            return ret;
        }
    }

    info->mtime = statbuf.st_mtim;
    info->atime = statbuf.st_atim;
    info->ctime = statbuf.st_ctim;
    SET_TIMESPEC_EMPTY(&info->btime);
    return 0;
}

#endif
