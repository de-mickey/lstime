// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDMUNIT_H
#define DDMUNIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
// Name: ddmunit
// Author: De Mickey
// Originally based on and inspired by minunit.h
// To install, just make the files ddmunit.h and ddmunit.c available
//
// With ddmunit, a testcase is simply a function returning true or
// false (success or failure).  Each test case contains one or more
// assertions.  The first assertion to fail outputs diagnostic info,
// and then terminates the testcase by returning false.  After its
// assertions the testcase function should return true.
//
// As each testcase terminates, its failure or success can be recorded
// by calling du_add(), which increments counters for later summaries.
// These summaries are output by calling du_suite_summary() and
// du_total_summary().
//
// To support larger programs, testcases can be optionally grouped
// into test suites.  Each test suite should be terminated by a call
// to du_suite_summary().  After all the suites/testcases,
// du_total_summary() should be called to output the final total
// counts.
//
// ddmunit provides two APIs, based on how state is handled.
// The 'simple' API keeps state in a global external variable.
// It requires defining and initializing a global variable named
// 'du_global'.  For example:

#if 0  // Simple API sample
#include "ddmunit.h"

du_state_t *du_global = NULL;

static bool test_something(void) {
    du_assert_int_eq(some_int(), 42, "what some_int is trying");
    du_assert_str_eq(some_str(), "expected", "what some_str is trying");
    return true;
}

int main(void) {
    du_global = du_alloc_state();
    // ...
    du_add(test_something());
    // ...
    int rc = du_total_summary("My Test Counts");
    exit(rc);
}
#endif

// The 'reentrant' API requires explicitly passing the state to ddmunit
// operations, but avoids using the global variable.  This approach
// likely means passing state down through the call stack to each
// testcase.  Note that the reentrant operations are named "*_r" and
// have an additional first argument "du_state_t *du".

#if 0  // Reentrant API sample
#include "ddmunit.h"

static bool test_thing(du_state_t *du) {
    du_assert_true_r(du, some_int() == 42, "what some_int is trying");
    du_assert_str_eq_r(du, some_str(), "expected", "what some_str is trying");
    return true;
}

int main(void) {
    du_state_t *du = du_alloc_state();
    // ...
    du_add_r(du, test_thing(du));
    // ...
    int rc = du_total_summary_r(du, "My Test Counts");
    du_free_state_r(du);
    exit(rc);
}
#endif

// The macros:
//
//   du_assert_true()     du_assert_true_r()
//   du_assert_str_eq()   du_assert_str_eq_r()
//   du_assert_int_eq()   du_assert_int_eq_r()
//
// all have a trailing ... for 1 to N additional arguments.  The first
// of these is 'msg_fmt', which is a required printf() format string.
// The remaining args are optional, depending on the 'msg_fmt'.  The
// resulting formatted message is only output if the assert fails.
// The recommendation is to provide a succint description of the
// particular test, at a minimum.  Then additional args can supply
// helpful info to understand and resolve failures, which reduce the
// need to clutter code with fprintf(stderr, ...) messages.  Note the
// assert will supply a trailing newline.
//
//   int rc = thing(arg);
//   du_assert_true(rc == 0, "thing(arg=\"%s\") returned rc=%d", arg, rc);
//
/////////////////////////////////////////////////////////////////////////


// Simple API macros hide the use of global state
#define du_assert_true(assertion, ...) \
    du_assert_true_r(du_global, (assertion), __VA_ARGS__)
#define du_assert_str_eq(act, exp, ...) \
    du_assert_str_eq_r(du_global, (act), (exp), __VA_ARGS__)
#define du_assert_int_eq(act, exp, ...) \
    du_assert_int_eq_r(du_global, (act), (exp), __VA_ARGS__)
#define du_add(success) du_add_r(du_global, (success))
#define du_suite_summary(label) du_suite_summary_r(du_global, (label))
#define du_total_summary(label) du_total_summary_r(du_global, (label))
#define du_set_fp(fp) du_set_fp_r(du_global, (fp))
#define du_show_success(flag) du_show_success_r(du_global, (flag))
#define du_free_state() du_free_state_r(du_global)

// Reentrant macros
#define du_assert_true_r(du, assertion, ...)     \
    if (!du_output_msg_r(                   \
            (du),                           \
            (assertion),                    \
            __FILE__,                       \
            __func__,                       \
            __LINE__,                       \
            #assertion,                     \
            __VA_ARGS__)) {                 \
        return false;                       \
    }

#define du_assert_str_eq_r(du, act, exp, ...)   \
    if (!du_output_msg_r(                       \
            (du),                               \
            du_safe_strcmp((act), (exp)) == 0,  \
            __FILE__,                           \
            __func__,                           \
            __LINE__,                           \
            du_fmt_str_eq(                      \
                (du)->temp_buf,                 \
                sizeof((du)->temp_buf),         \
                (act),                          \
                (exp)),                         \
            __VA_ARGS__)) {                     \
        return false;                           \
    }

#define du_assert_int_eq_r(du, act, exp, ...)   \
    if (!du_output_msg_r(                       \
            (du),                               \
            (act) == (exp),  \
            __FILE__,                           \
            __func__,                           \
            __LINE__,                           \
            du_fmt_int_eq(                      \
                (du)->temp_buf,                 \
                sizeof((du)->temp_buf),         \
                (act),                          \
                (exp)),                         \
            __VA_ARGS__)) {                     \
        return false;                           \
    }


// container for the needed state
typedef struct du_state {
    int total_tests_run;
    int total_tests_failed;
    int suite_tests_run;
    int suite_tests_failed;
    FILE *fp;  // set to stdout in du_alloc_state()
    bool show_success;
    char temp_buf[512];
} du_state_t;

// global used by Simple API
extern du_state_t *du_global;


// needed by both Simple and Reentrant APIs
du_state_t *du_alloc_state(void); // allocate and initialize

// Reentrant functions
void du_add_r(du_state_t *du, bool success);
int du_suite_summary_r(du_state_t *du, const char *suite_label);
int du_total_summary_r(du_state_t *du, const char *total_label);
void du_set_fp_r(du_state_t *du, FILE *fp);  // override output stream
void du_show_success_r(du_state_t *du, bool flag);
void du_free_state_r(du_state_t *du);

// internal helper utils
__attribute__((__format__(__printf__, 7, 8)))
bool du_output_msg_r(du_state_t *du,
                     bool success,
                     const char *file,
                     const char *func,
                     int line,
                     const char *assertion,
                     const char *msg_fmt, ...);
char *du_fmt_str_eq(char *buf, size_t bufsize,
                    const char *act, const char *exp);
int du_safe_strcmp(const char *left, const char *right);
char *du_fmt_int_eq(char *buf, size_t bufsize,
                    long long act, long long exp);

#endif
