// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lstime_private.h"


// note descending time is the default for COMP_TIME
// ascending time is the reverse 
#define COMP_TIME(NAME, T1, T2)                         \
    static int NAME(const void *v1, const void *v2) {   \
        const lstime_info *info1 = v1;                  \
        const lstime_info *info2 = v2;                  \
        return comp_timespec((T2), (T1));               \
}

#define COMP_PATH(NAME, P1, P2)                        \
    static int NAME(const void *v1, const void *v2) {  \
        const lstime_info *info1 = v1;                 \
        const lstime_info *info2 = v2;                 \
        return strcmp((P1), (P2));                     \
}

static int comp_timespec(const timespec *t1, const timespec *t2) {
    if (t1->tv_sec > t2->tv_sec) {
        return 1;
    } else if (t1->tv_sec < t2->tv_sec) {
        return -1;
    }
    if (t1->tv_nsec > t2->tv_nsec) {
        return 1;
    } else if (t1->tv_nsec < t2->tv_nsec) {
        return -1;
    }
    return 0;
}

COMP_TIME(comp_m_fwd, &info1->mtime, &info2->mtime)
COMP_TIME(comp_m_rev, &info2->mtime, &info1->mtime)
COMP_TIME(comp_a_fwd, &info1->atime, &info2->atime)
COMP_TIME(comp_a_rev, &info2->atime, &info1->atime)
COMP_TIME(comp_c_fwd, &info1->ctime, &info2->ctime)
COMP_TIME(comp_c_rev, &info2->ctime, &info1->ctime)
COMP_TIME(comp_b_fwd, &info1->btime, &info2->btime)
COMP_TIME(comp_b_rev, &info2->btime, &info1->btime)

COMP_PATH(comp_p_fwd, info1->sortkey, info2->sortkey)
COMP_PATH(comp_p_rev, info2->sortkey, info1->sortkey)

void lstime_sort_list(arr_wrapper *list, const lstime_options *opts) {
    if (list->num_elems == 0) {
        return;
    }

    static char buf[MAX_PATH_LEN];
    if (opts->sort_field == 'p') {
        // populate sortkey
        lstime_info *ptr = list->arr;
        lstime_info *end = ptr + list->num_elems;
        for ( ; ptr < end ; ++ptr) {
            size_t n = strxfrm(buf, ptr->path, sizeof(buf));
            if (n >= sizeof(buf)) {
                err("strxfrm exceeded buf len: %zu", sizeof(buf));
                exit(39);
            }
            ptr->sortkey = strdup(buf);
        }
    }

    int (*comp)(const void *, const void *) = NULL;
    if (opts->reverse) {
        if (opts->sort_field == 'm') {
            comp = comp_m_rev;
        } else if (opts->sort_field == 'a') {
            comp = comp_a_rev;
        } else if (opts->sort_field == 'c') {
            comp = comp_c_rev;
        } else if (opts->sort_field == 'b') {
            comp = comp_b_rev;
        } else if (opts->sort_field == 'p') {
            comp = comp_p_rev;
        } else {
            // n[one] should not appear here
            err("sort_list: logic error / bad state");
            exit(34);
        }
    } else {
        if (opts->sort_field == 'm') {
            comp = comp_m_fwd;
        } else if (opts->sort_field == 'a') {
            comp = comp_a_fwd;
        } else if (opts->sort_field == 'c') {
            comp = comp_c_fwd;
        } else if (opts->sort_field == 'b') {
            comp = comp_b_fwd;
        } else if (opts->sort_field == 'p') {
            comp = comp_p_fwd;
        } else {
            // n[one] should not appear here
            err("sort_list: logic error / bad state");
            exit(35);
        }

    }
    qsort(list->arr, list->num_elems, sizeof(lstime_info), comp);
}

