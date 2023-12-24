// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LSTIME_PRIVATE_H
#define LSTIME_PRIVATE_H 1

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lstime.h"

#define MAX_PATH_LEN 8192
#define MAX_TIME_LEN 1024

#define SET_TIMESPEC_EMPTY(ts_ptr) \
    { (ts_ptr)->tv_sec = -1; (ts_ptr)->tv_nsec = -1; }
#define HAS_TIMESPEC(ts_ptr) \
    ((ts_ptr)->tv_sec != -1 && (ts_ptr)->tv_nsec != -1)

#define err(...) lstime_err(__VA_ARGS__)
#define warn(...) lstime_warn(__VA_ARGS__)
#define msg(...) lstime_msg(__VA_ARGS__)

iconv_t lstime_iconv_init(void);  // not really needed, as called automatically
bool lstime_iconv(const char *path, bool debug);
uint32_t *lstime_outbuf(void);
size_t lstime_outbuf_len(void);
size_t lstime_outbuf_capacity(void);
void lstime_iconv_finit(void);    // free up some resources
void lstime_set_prog(const char *pgm);  // for lstime_msg messages
const char *lstime_get_prog(void);
__attribute__((__format__(__printf__, 1, 2)))
void lstime_warn(const char *fmt, ...);
__attribute__((__format__(__printf__, 1, 2)))
void lstime_err(const char *fmt, ...);
__attribute__((__format__(__printf__, 2, 3)))
void lstime_msg(const char *level, const char *fmt, ...);
void lstime_msg_ap(const char *level, const char *fmt, va_list ap);

#endif
