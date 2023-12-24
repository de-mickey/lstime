// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lstime_private.h"
#include "lstime_tests.h"

static char *mem_buf = NULL;
static size_t mem_buf_size = 0;

static FILE *open_mem(void) {
    FILE *fp = open_memstream(&mem_buf, &mem_buf_size);
    if (fp == NULL) {
        err("open_memstream failed: %s", strerror(errno));
        exit(29);
    }
    return fp;
}

static const char *close_and_get_mem(FILE *fp) {
    if (fclose(fp) != 0) {
        err("fclose on memstream failed: %s", strerror(errno));
        exit(30);
    }
    return mem_buf;
}

static void free_mem(void) {
    free(mem_buf);
    mem_buf = NULL;
    mem_buf_size = 0;
}


static bool test_mtime(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";
    info.mtime.tv_sec = 86400;
    info.mtime.tv_nsec = 123456789;

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%m", "%F %9N", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "1970-01-02 123456789", "%%m");
    free_mem();
    return true;
}

static bool test_atime(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";
    info.atime.tv_sec = 172800;
    info.atime.tv_nsec = 987654321;

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%a", "%F %9N", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "1970-01-03 987654321", "%%a");
    free_mem();
    return true;
}

static bool test_ctime(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";
    info.ctime.tv_sec = 120;
    info.ctime.tv_nsec = 987123000;

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%c", "%T %6N", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "00:02:00 987123", "%%c");
    free_mem();
    return true;
}

static bool test_btime(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";
    info.btime.tv_sec = 300;
    info.btime.tv_nsec = 999999999;
    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%b", "%T %3N", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "00:05:00 999", "%%b");
    free_mem();
    return true;
}

static bool test_raw_path(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "\t\u03B1";

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%r", "", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "\t\u03B1", "%%r");
    free_mem();
    return true;
}

static bool test_escaped_path(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "\t\u03B1";

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%p", "", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "$'\\t\u03B1'", "%%p");
    free_mem();
    return true;
}

static bool test_u_escaped_path(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "\t\u03B1";

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%u", "", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "$'\\t\\u03B1'", "%%u");
    free_mem();
    return true;
}

static bool test_zero(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%z%z ", "", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_true(str[0] =='\0' &&
                   str[1] == '\0' &&
                   str[2] == ' ' &&
                   str[3] == '\0',
                   "%%z zero bytes");
    free_mem();
    return true;
}

static bool test_newline(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%n%n", "", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "\n\n", "%%n newlines");
    free_mem();
    return true;
}

static bool test_percentile(void) {
    lstime_info info;
    memset(&info, 0, sizeof(info));
    info.path = "no path";

    FILE *fp = open_mem();
    lstime_out_it(fp, &info, "%%%%", "", true, false);
    const char *str = close_and_get_mem(fp);
    du_assert_str_eq(str, "%%", "%%%% percentiles");
    free_mem();
    return true;
}

int output_item_suite(void) {
    du_add(test_mtime());
    du_add(test_atime());
    du_add(test_ctime());
    du_add(test_btime());
    du_add(test_raw_path());
    du_add(test_escaped_path());
    du_add(test_u_escaped_path());
    du_add(test_zero());
    du_add(test_newline());
    du_add(test_percentile());
    return du_suite_summary("lstime_output_item Test Suite Summary");
}

