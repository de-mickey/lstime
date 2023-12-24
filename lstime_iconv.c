// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <errno.h>

#include "lstime_private.h"

// note: not using plain "UTF-32" as that generates BOMs
static const char *dest_encoding =
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        "UTF-32BE";
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        "UTF-32LE";
#else
#error "Could not detect byte order for encoding"
#endif

static iconv_t to_utf32 = (iconv_t) -1;
static uint32_t outbuf[MAX_PATH_LEN];
static size_t outbuf_len = 0;

iconv_t lstime_iconv_init(void) {
    if (to_utf32 == (iconv_t)-1) {
        to_utf32 = iconv_open(dest_encoding, "UTF-8");
        if (to_utf32 == (iconv_t)-1) {
            err("iconv_open failed: dest encoding %s: %s",
                dest_encoding, strerror(errno));
            exit(22);
        }
    }
    return to_utf32;
}

void lstime_iconv_finit(void) {
    if (to_utf32 != (iconv_t)-1) {
        if (iconv_close(to_utf32) != 0) {
            err("iconv_close() failed: %s", strerror(errno));
            exit(19);
        }
        to_utf32 = (iconv_t) -1;
    }
}

bool lstime_iconv(const char *path, bool debug) {
    char *in_ptr = (char *) path;
    size_t in_size_left = strlen(path);
    char *out_ptr = (char *) outbuf;
    size_t out_size_left = sizeof(outbuf);

    size_t rc = iconv(lstime_iconv_init(),
                      &in_ptr, &in_size_left,
                      &out_ptr, &out_size_left);

    if (rc == (size_t)-1 || in_size_left != 0 || rc != 0) {
        // problem with UTF-8, likely unexpected binary
        if (debug) {
            if (rc == (size_t)-1) {
                warn("iconv conversion failed: %s", strerror(errno));
            } else if (in_size_left != 0) {
                warn("iconv conversion incomplete: %s: remaining=%zu",
                     strerror(errno), in_size_left);
            } else if (rc != 0) {
                warn("iconv substituted characters: %zu\n", rc);
            }
        }
        outbuf_len = 0;
        return false;
    }
    // conversion successful
    outbuf_len = (uint32_t *)out_ptr - outbuf;
    return true;
}

uint32_t *lstime_outbuf(void) {
    return outbuf;
}

size_t lstime_outbuf_len(void) {
    return outbuf_len;
}

size_t lstime_outbuf_capacity(void) {
    return sizeof(outbuf) / sizeof(outbuf[0]);
}




