// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <ctype.h>

#include "lstime_private.h"

static void tm_from_timespec(struct tm *tm, timespec ts, bool use_utc) {
    time_t timet = ts.tv_sec;
    if (use_utc) {
        if (!gmtime_r(&timet, tm)) {
            err("gmtime_r: %s", strerror(errno));
            exit(5);
        }
    } else {
        if (!localtime_r(&timet, tm)) {
            err("localtime_r: %s", strerror(errno));
            exit(6);
        }
    }
}


static char *preprocess_time_format(const char *time_format,
                                    long nsec,
                                    struct tm *tm) {
    static char buf[MAX_TIME_LEN];
    char *buf_ptr = buf;
    char *buf_end = buf + sizeof(buf) - 1;
    const char *ptr = time_format;

    while (*ptr != '\0') {
        if (*ptr == '%') {
            const char *spec_beg = ptr++;
            int extended_spec_found = 0;
            do { // posit we will eventually find a valid extended spec
                // flags
                while (*ptr != '\0' && strchr("_-0^#", *ptr) != NULL) {
                    ++ptr;
                }
                if (*ptr == '\0') {
                    break;
                }

                // width
                const char *width_beg = ptr;
                while (*ptr != '\0' && isdigit((unsigned char) *ptr)) {
                    ++ptr;
                }
                const char *width_end = ptr;
                if (*ptr == '\0') {
                    break;
                }

                // modifier
                if (*ptr == 'E' || *ptr == 'O') {
                    ++ptr;
                }
                if (*ptr == '\0') {
                    break;
                }

                // spec letter
                // if (!(isalpha((unsigned char) *ptr) || *ptr == '%' || *ptr == '+')) {
                //     break;
                // }

                const char spec_letter = *ptr++;
                if (spec_letter == 'N') {
                    // confirmed we have an extended spec
                    extended_spec_found = 1;
                    int nano_width = 9;  // default & max
                    if (width_beg + 1 == width_end) {
                        nano_width = *width_beg - '0';
                    }
                    // fprintf(stderr, "nano_width = %d\n", nano_width);
                    if (buf_ptr > buf_end - 10) {
                        err("preprocess_time_format: buf len %zu exceeded",
                            sizeof(buf));
                        exit(16);
                    }
                    size_t rc = snprintf(buf_ptr, 11, "%09ld", nsec);
                    if (rc != 9) {
                        err("preprocess_time_format: internal logic error");
                        exit(21);
                    }
                    buf_ptr += nano_width;
                } else if (spec_letter == ':' && *ptr == 'z') {
                    char zbuf[12];
                    // confirmed we have an extended spec
                    extended_spec_found = 1;
                    ++ptr;
                    int len = strftime(zbuf, sizeof(zbuf), "%z", tm);
                    if (len == 0) {
                        err("strftime: zbuf: len %zu exhausted", sizeof(zbuf));
                        exit(11);
                    } else if (len != 5) {
                        err("strftime: unexpected result: len=%d", len);
                        exit(31);
                    }
                    if (buf_ptr > buf_end - 7) {
                        err("preprocess_time_format: exceeded buf len %zu",
                            sizeof(buf));
                        exit(18);
                    }
                    // now copy chars and insert colon
                    *buf_ptr++ = zbuf[0];
                    *buf_ptr++ = zbuf[1];
                    *buf_ptr++ = zbuf[2];
                    *buf_ptr++ = ':';
                    *buf_ptr++ = zbuf[3];
                    *buf_ptr++ = zbuf[4];
                }
            } while (0);

            if (!extended_spec_found) {
                // native strftime spec or something unparseable
                if (buf_ptr > buf_end - (ptr - spec_beg)) {
                    err("preprocess_time_format: exceeded buf len %zu",
                          sizeof(buf));
                    exit(17);
                }
                while (spec_beg < ptr) {
                    *buf_ptr++ = *spec_beg++;
                }
            }

        } else { // plain literal char
            if (buf_ptr > buf_end - 1) {
                err("preprocess_time_format: exceeded buf len %zu",
                    sizeof(buf));
                exit(20);
            }
            *buf_ptr++ = *ptr++;
        }
    }

    *buf_ptr = '\0';
    return buf;
}


const char *lstime_format_timestamp(timespec ts,
                                    const char *time_format,
                                    bool format_time_as_utc) {
    static char buffer[MAX_TIME_LEN];
    struct tm tm;

    if (! HAS_TIMESPEC(&ts)) {
        return "N/A";
    }
    tm_from_timespec(&tm, ts, format_time_as_utc);

    const char *tmpfmt = preprocess_time_format(time_format, ts.tv_nsec, &tm);
    int len = strftime(buffer, sizeof(buffer), tmpfmt, &tm);
    if (len == 0) {
        err("strftime: buffer: len %zu exhausted", sizeof(buffer));
        exit(23);
    }
    return buffer;
}

