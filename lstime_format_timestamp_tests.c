// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lstime_private.h"
#include "lstime_tests.h"

static bool test_unix_epoch() {
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    const char *rc = lstime_format_timestamp(ts, "%FT%T.%9N", true);
    du_assert_str_eq(rc, "1970-01-01T00:00:00.000000000",
              "unix epoch");
    return true;
}

static bool test_local_unix_epoch() {
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 123000;
    const char *rc = lstime_format_timestamp(ts, "%FT%T.%9N%z", false);
    du_assert_str_eq(rc, "1969-12-31T22:00:00.000123000-0200",
              "local unix epoch + 123 usec");
    return true;
}

static bool test_msec() {
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1999999;
    const char *rc = lstime_format_timestamp(ts, "%FT%T.%3N%:z", false);
    du_assert_str_eq(rc, "1969-12-31T22:00:00.001-02:00",
              "local unix epoch + truncated to 1 msec");
    return true;
}

static bool test_year_2038_problem() {
    // datetime when signed 32-bit time_t rolls over to negative (1901)
    timespec ts;
    ts.tv_sec = 2147483648; // 2**31
    ts.tv_nsec = 0;
    const char *rc = lstime_format_timestamp(ts, "%FT%T%:z", true);
    du_assert_str_eq(rc, "2038-01-19T03:14:08+00:00",
              "Y2038 date for time_t rollover bug");
    // no bug on systems with a 64-bit time_t
    return true;
}

static bool test_timezone() {
    timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 111333999;
    const char *rc = lstime_format_timestamp(ts, "%:z%:z%3N%:z", false);
    du_assert_str_eq(rc, "-02:00-02:00111-02:00", "%%:z with %%3N test");
    return true;
}


static void build_info_by_time(arr_wrapper *list, int type, long sec, long nsec) {
    timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";
    if (type == 'm') {
        info.mtime = ts;
    } else if (type == 'a') {
        info.atime = ts;
    } else if (type == 'c') {
        info.ctime = ts;
    } else if (type == 'b') {
        info.btime = ts;
    } else {
        err("not a valid info type");
        exit(2);
    }
    add_info_to_list(list, &info);
}

static void build_info_by_path(arr_wrapper *list, const char *path) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = strdup(path);
    add_info_to_list(list, &info);
}


static bool test_fwd_m_sort() {
    arr_wrapper list;
    memset(&list, 0, sizeof(list));
    lstime_options opts;
    lstime_set_option_defaults(&opts);
    opts.reverse = false;
    opts.sort_field = 'm';

    build_info_by_time(&list, 'm', 0, 3);
    build_info_by_time(&list, 'm', 0, 4);
    build_info_by_time(&list, 'm', 0, 1);
    build_info_by_time(&list, 'm', 0, 2);

    lstime_sort_list(&list, &opts);

    du_assert_int_eq(list.arr[0].mtime.tv_nsec, 4, " ");
    du_assert_int_eq(list.arr[1].mtime.tv_nsec, 3, " ");
    du_assert_int_eq(list.arr[2].mtime.tv_nsec, 2, " ");
    du_assert_int_eq(list.arr[3].mtime.tv_nsec, 1, " ");
    return true;
}

static bool test_rev_a_sort() {
    arr_wrapper list;
    memset(&list, 0, sizeof(list));
    lstime_options opts;
    lstime_set_option_defaults(&opts);
    opts.reverse = true;
    opts.sort_field = 'a';

    build_info_by_time(&list, 'a', 3, 0);
    build_info_by_time(&list, 'a', 4, 0);
    build_info_by_time(&list, 'a', 1, 0);
    build_info_by_time(&list, 'a', 2, 0);

    lstime_sort_list(&list, &opts);

    du_assert_int_eq(list.arr[0].atime.tv_sec, 1, " ");
    du_assert_int_eq(list.arr[1].atime.tv_sec, 2, " ");
    du_assert_int_eq(list.arr[2].atime.tv_sec, 3, " ");
    du_assert_int_eq(list.arr[3].atime.tv_sec, 4, " ");
    return true;
}

static bool test_fwd_p_sort() {
    arr_wrapper list;
    memset(&list, 0, sizeof(list));
    lstime_options opts;
    lstime_set_option_defaults(&opts);
    opts.reverse = false;
    opts.sort_field = 'p';

    build_info_by_path(&list, "bbb");
    build_info_by_path(&list, "ddd");
    build_info_by_path(&list, "aaa");
    build_info_by_path(&list, "ccc");

    lstime_sort_list(&list, &opts);
    // lstime_output_list(stderr, &list, &opts);

    du_assert_str_eq(list.arr[0].path, "aaa", " ");
    du_assert_str_eq(list.arr[1].path, "bbb", " ");
    du_assert_str_eq(list.arr[2].path, "ccc", " ");
    du_assert_str_eq(list.arr[3].path, "ddd", " ");
    return true;
}

static bool test_rev_p_sort() {
    arr_wrapper list;
    memset(&list, 0, sizeof(list));
    lstime_options opts;
    lstime_set_option_defaults(&opts);
    opts.reverse = true;
    opts.sort_field = 'p';

    build_info_by_path(&list, "bbb");
    build_info_by_path(&list, "ddd");
    build_info_by_path(&list, "aaa");
    build_info_by_path(&list, "ccc");

    lstime_sort_list(&list, &opts);
    // lstime_output_list(stderr, &list, &opts);

    du_assert_str_eq(list.arr[0].path, "ddd", " ");
    du_assert_str_eq(list.arr[1].path, "ccc", " ");
    du_assert_str_eq(list.arr[2].path, "bbb", " ");
    du_assert_str_eq(list.arr[3].path, "aaa", " ");
    return true;
}



int format_timestamp_suite(void) {
    du_add(test_unix_epoch());
    du_add(test_local_unix_epoch());
    du_add(test_msec());
    du_add(test_year_2038_problem());
    du_add(test_timezone());
    du_add(test_fwd_m_sort());
    du_add(test_rev_a_sort());
    du_add(test_fwd_p_sort());
    du_add(test_rev_p_sort());
    return du_suite_summary("lstime_format_timestamp Test Suite Summary");
}
