// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lstime.h"
#include "lstime_tests.h"


static bool test_plain() {
    const char *rc = lstime_format_path("abc_def.ghi", false, false);
    du_assert_str_eq(rc, "abc_def.ghi", "plain LEVEL1 path");
    return true;
}

static bool test_plain_2() {
    const char *rc = lstime_format_path("xyz/def:ghi", false, false);
    du_assert_str_eq(rc, "xyz/def:ghi", "another plain LEVEL1 path");
    return true;
}

static bool test_shell_meta() {
    const char *rc = lstime_format_path("abc$def", false, false);
    du_assert_str_eq(rc, "'abc$def'", "shell meta LEVEL2 path");
    return true;
}

static bool test_shell_meta_2() {
    const char *rc = lstime_format_path("abc*def", false, false);
    du_assert_str_eq(rc, "'abc*def'", "another shell meta LEVEL2 path");
    return true;
}

static bool test_u_escapes() {
    const char *rc = lstime_format_path("\u03B0\u03B1\u03B2", false, false);
    du_assert_str_eq(rc, "'\u03B0\u03B1\u03B2'", "utf-8 LEVEL3 path");
    return true;
}

static bool test_u_escapes_2() {
    const char *rc = lstime_format_path("\u03B0\u03B1\u03B2", true, false);
    du_assert_str_eq(rc, "$'\\u03B0\\u03B1\\u03B2'", "utf-8 LEVEL5 path");
    return true;
}

static bool test_control_codes() {
    const char *rc = lstime_format_path("abc\tdef", false, false);
    du_assert_str_eq(rc, "$'abc\\tdef'",
                     "control codes LEVEL4 paths");
    return true;
}

static bool test_control_codes_2() {
    // special case: single quote also provokes LEVEL4
    const char *rc = lstime_format_path("abc'def", false, false);
    du_assert_str_eq(rc, "$'abc\\'def'", "control codes LEVEL4 paths");
    return true;
}

static bool test_binary() {
    // when invalid UTF-8 sequences are found
    const char *rc = lstime_format_path("abc\xFF\xFF\xFF" "def", false, false);
    du_assert_str_eq(rc, "$'abc\\xFF\\xFF\\xFFdef'",
                     "binary LEVEL6 paths");
    return true;
}

static bool test_binary_2() {
    const char *rc = lstime_format_path("\u03B1\xFF", false, false);
    du_assert_str_eq(rc, "$'\\xCE\\xB1\\xFF'",
                     "binary LEVEL6 paths");
    return true;
}

int format_path_suite(void) {
    du_add(test_plain());
    du_add(test_plain_2());
    du_add(test_shell_meta());
    du_add(test_shell_meta_2());
    du_add(test_u_escapes());
    du_add(test_u_escapes_2());
    du_add(test_control_codes());
    du_add(test_control_codes_2());
    du_add(test_binary());
    du_add(test_binary_2());
    return du_suite_summary("lstime_format_path Test Suite Summary");
}
