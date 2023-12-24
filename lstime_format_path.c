// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdint.h>
#include <errno.h>

#include "lstime_private.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define LEVEL1 1  // no quoting needed, just graphic ASCII
#define LEVEL2 2  // has space or shell specials, single-quotes needed
#define LEVEL3 3  // has multi-byte UTF-8, single-quotes needed
#define LEVEL4 4  // has single quote or control codes, escapes needed
#define LEVEL5 5  // user requested escapes for codepoints beyond 7-bit ASCII
#define LEVEL6 6  // invalid UTF-8 or raw binary, hex escapes needed

static const char *hex = "0123456789ABCDEF";

static int copy_bytes_to_cps(uint32_t *cps, size_t capacity, const char *str) {
    size_t len = strlen(str);
    if (len > capacity) {
        err("copy_bytes_to_cps: cps buffer cannot hold %zu bytes", len);
        exit(27);
    }

    while (*str != '\0') {
        *cps++ = (*str++ & 0xFF);
    }
    return len;
}

static char *bash_u_escape(wchar_t cp) {
    static char buf[12];
    const char *fmt;
    if (cp <= 0xFFFF) {
        fmt = "\\u%04X";
    } else {
        fmt = "\\U%08X";
    }
    sprintf(buf, fmt, cp);
    return buf;
}

static int parse_for_display_level(const char *path, int min_level, bool escape_uni) {
    int level = min_level;
    for (const char *ptr = path; *ptr != '\0'; ptr++) {
        unsigned char c = (unsigned char) *ptr;
        switch (c) {
        case ' ':
        case '\"':
        case '|':
        case '&':
        case ';':
        case '(':
        case ')':
        case '<':
        case '>':
        case '{':
        case '}':
        case '!':
        case '$':
        case '`':
        case '\\':
        case '*':
        case '?':
        case '[':
        case ']':
            // shell meta and special chars better off single quoted
            level = MAX(level, LEVEL2);
            break;
        case '\'':
            // the only printable char not allowed in single quotes
            level = MAX(level, LEVEL4);
            break;
        default:
            if (c < 0x20 || c == 0x7F) {
                // control codes need escapes
                level = MAX(level, LEVEL4);
            } else if (c < 0x80) {
                // remaining plain printable ASCII fall through
            } else {
                // for now, assumed to be UTF-8 multi-byte code points
                if (escape_uni) {
                    level = MAX(level, LEVEL5);
                } else {
                    level = MAX(level, LEVEL3);
                }
            }
            break;
        }
    }
    return level;
}

static const char *bash_quote_cps(const uint32_t *cps,
                                  size_t cps_len,
                                  int disp_level) {
    static char quoted_buf[MAX_PATH_LEN];
    char *qptr = quoted_buf;
    uint32_t cp;

    if (disp_level >= LEVEL4) {
        *qptr++ = '$';
    }
    if (disp_level >= LEVEL2) {
        *qptr++ = '\'';
    }

    while (cps_len-- > 0) {
        cp = *cps++;
        if ((size_t)(qptr - quoted_buf) > sizeof(quoted_buf) - 12) {
            // \UHHHHHHHH + close quote + nul == 12
            // 12 more bytes might be needed
            err("bash_quote_cps: quoted_buf len %zu exceeded", sizeof(quoted_buf));
            exit(13);
        }

        if (disp_level <= LEVEL3) {
            *qptr++ = cp;
        } else {  // >= LEVEL4
            switch (cp) {
            case '\a': *qptr++ = '\\'; *qptr++ = 'a'; break;
            case '\b': *qptr++ = '\\'; *qptr++ = 'b'; break;
            case 033:  *qptr++ = '\\'; *qptr++ = 'E'; break;
            case '\f': *qptr++ = '\\'; *qptr++ = 'f'; break;
            case '\n': *qptr++ = '\\'; *qptr++ = 'n'; break;
            case '\r': *qptr++ = '\\'; *qptr++ = 'r'; break;
            case '\t': *qptr++ = '\\'; *qptr++ = 't'; break;
            case '\v': *qptr++ = '\\'; *qptr++ = 'v'; break;
            case '\\': *qptr++ = '\\'; *qptr++ = '\\'; break;
            case '\'': *qptr++ = '\\'; *qptr++ = '\''; break;
            case '\"': *qptr++ = '\\'; *qptr++ = '\"'; break;
            // case '\?': *qptr++ = '\\'; *qptr++ = '\?'; break;
            default:
                if (cp >= 0x20 && cp <= 0x7E) {
                    *qptr++ = cp;
                } else {
                    if (disp_level == LEVEL4) {
                        if (cp <= 0x7F) {
                            // control codes escaped
                            strcpy(qptr, bash_u_escape(cp));
                            qptr += strlen(qptr);
                        } else {
                            // pass thru multi-byte UTF-8
                            *qptr++ = cp;
                        }
                    } else if (disp_level == LEVEL5) {
                        // control codes and multi-byte UTF-8 escaped
                        strcpy(qptr, bash_u_escape(cp));
                        qptr += strlen(qptr);
                    } else if (disp_level == LEVEL6) {
                        // display all non-ASCII as hex
                        *qptr++ = '\\';
                        *qptr++ = 'x';
                        *qptr++ = hex[cp >> 4];
                        *qptr++ = hex[cp & 0x0F];
                    } else {
                    }
                }
                break;
            }
        }
    }

    if (disp_level >= LEVEL2) {
        *qptr++ = '\'';
    }
    *qptr++ = '\0';
    return quoted_buf;
}

const char *lstime_format_path(const char *path, bool escape_uni, bool debug) {
    int disp_level = parse_for_display_level(path, LEVEL1, escape_uni);
    uint32_t *cps_buf = lstime_outbuf();
    size_t cps_len = 0;

    if (disp_level >= LEVEL3 || disp_level <= LEVEL5) {
        // conversion to UTF-32 verifies UTF-8 is valid
        // and provides code point values for escapes
        if (lstime_iconv(path, debug)) {
            // successful
            cps_len = lstime_outbuf_len();
        } else {
            // problem with UTF-8, likely unexpected binary
            // so failover and display as all hex
            disp_level = MAX(disp_level, LEVEL6);
        }
    }

    if (disp_level != LEVEL5) {
        // only LEVEL5 actually needs the UTF-32 values
        // the others can be formatted from the original byte values
        cps_len = copy_bytes_to_cps(cps_buf, lstime_outbuf_capacity(), path);
    }

    return bash_quote_cps(cps_buf, cps_len, disp_level);
}

