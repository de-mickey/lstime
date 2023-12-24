// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdarg.h>

#include "lstime_private.h"

static const char *msg_prog = "";

void lstime_set_prog(const char *pgm) {
    msg_prog = pgm;
}

const char *lstime_get_prog(void) {
    return msg_prog;
}

void lstime_warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lstime_msg_ap("warning", fmt, ap);
    va_end(ap);
}

void lstime_err(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lstime_msg_ap("error", fmt, ap);
    va_end(ap);
}

void lstime_msg(const char *level, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    lstime_msg_ap(level, fmt, ap);
    va_end(ap);
}

void lstime_msg_ap(const char *level, const char *fmt, va_list ap) {
    fprintf(stderr, "%s %s: ", msg_prog, level);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

