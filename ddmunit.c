// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddmunit.h"

du_state_t *du_alloc_state(void) {
    du_state_t *du = malloc(sizeof(du_state_t));
    if (du != NULL) {
        du->total_tests_run = 0;
        du->total_tests_failed = 0;
        du->suite_tests_run = 0;
        du->suite_tests_failed = 0;
        du->fp = stderr;
        du->show_success = false;
        du->temp_buf[0] = '\0';
    }
    return du;
}

bool du_output_msg_r(du_state_t *du,
                     bool success,
                     const char *file,
                     const char *func,
                     int line,
                     const char *assertion,
                     const char *msg_fmt, ...) {
    va_list ap;
    if (success && !du->show_success) {
        return success;
    }
    fprintf(du->fp, "%s %s line %d in %s():\n    %s\n    ",
            (success) ? "SUCCESS" : "FAILURE",
            file, line, func, assertion);
    va_start(ap, msg_fmt);
    vfprintf(du->fp, msg_fmt, ap);
    va_end(ap);
    fprintf(du->fp, "\n");
    return success;
}

void du_add_r(du_state_t *du, bool success) {
    ++du->total_tests_run;
    ++du->suite_tests_run;
    if (! success) {
        ++du->total_tests_failed;
        ++du->suite_tests_failed;
    }
}

int du_suite_summary_r(du_state_t *du, const char *suite_label) {
    if (suite_label == NULL) {
        suite_label = "Suite Summary:";
    }
    fprintf(du->fp, "--------  %s\n", suite_label);
    fprintf(du->fp, "%8d  tests run\n", du->suite_tests_run);
    fprintf(du->fp, "%8d  tests failed\n", du->suite_tests_failed);
    fprintf(du->fp, "--------\n");
    int rc = (du->suite_tests_run == 0 ||
              du->suite_tests_failed > 0) ? 1 : 0;
    // reset counters to start next suite:
    du->suite_tests_run = 0;
    du->suite_tests_failed = 0;
    // return value: 1 if any failures or no tests ran in this suite, else 0
    return rc;
}

int du_total_summary_r(du_state_t *du, const char *total_label) {
    if (total_label == NULL) {
        total_label = "Total Summary:";
    }
    fprintf(du->fp, "========  %s\n", total_label);
    fprintf(du->fp, "%8d  tests run\n", du->total_tests_run);
    fprintf(du->fp, "%8d  tests failed\n", du->total_tests_failed);
    fprintf(du->fp, "========\n");
    // return value: 1 if any failures or no tests ran, else 0
    int rc = (du->total_tests_run == 0 ||
              du->total_tests_failed > 0) ? 1 : 0;
    return rc;
}

void du_set_fp_r(du_state_t *du, FILE *fp) {
    du->fp = fp;
}

void du_show_success_r(du_state_t *du, bool flag) {
    du->show_success = flag;
}

void du_free_state_r(du_state_t *du) {
    free(du);
}

char *du_fmt_str_eq(char *buf, size_t bufsize,
                     const char *act, const char *exp) {
    // truncates result to bufsize, intentionally
    if (act == NULL) {
        if (exp == NULL) {
            snprintf(buf, bufsize, "NULL == NULL");
        } else {
            snprintf(buf, bufsize, "NULL == \"%s\"", exp);
        }
    } else if (exp == NULL) {
        snprintf(buf, bufsize, "\"%s\" == NULL", act);
    } else {
        snprintf(buf, bufsize, "\"%s\" == \"%s\"", act, exp);
    }
    return buf;
}

int du_safe_strcmp(const char *left, const char *right) {
    if (left == NULL) {
        return (right == NULL) ? 0 : -1;
    } else {
        return (right == NULL) ? 1 : strcmp(left, right);
    }
}

char *du_fmt_int_eq(char *buf, size_t bufsize,
                    long long act, long long exp) {
    snprintf(buf, bufsize, "%lld == %lld", act, exp);
    return buf;
}

