// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <langinfo.h>
#include <unistd.h>
#include <getopt.h>

#include "lstime_private.h"

#if 0
#define SHOW_CAT(cat)                                                   \
    fprintf(stderr, "%s=%s\n", #cat, null2invalid(setlocale(cat, NULL)))

static const char *null2invalid(const char *str) {
    return (str == NULL) ? "invalid" : str;
}

static void show_locale_settings() {
    // C standards:
    SHOW_CAT(LC_ALL);
    SHOW_CAT(LC_CTYPE);
    SHOW_CAT(LC_NUMERIC);
    SHOW_CAT(LC_TIME);
    SHOW_CAT(LC_COLLATE);
    SHOW_CAT(LC_MONETARY);

    // POSIX.1 standards
    SHOW_CAT(LC_MESSAGES);

    // GNU extensions:
    SHOW_CAT(LC_ADDRESS);
    SHOW_CAT(LC_IDENTIFICATION);
    SHOW_CAT(LC_MEASUREMENT);
    SHOW_CAT(LC_NAME);
    SHOW_CAT(LC_PAPER);
    SHOW_CAT(LC_TELEPHONE);

    fprintf(stderr, "\n");
}
#endif

static bool set_my_app_locale(const char *loc) {
    if (setlocale(LC_ALL, loc) == NULL) {
        err("setlocale failed: \"%s\": Check LANG, LC_CTYPE, LC_ALL", loc);
        // show_locale_settings();
        return false;
    }

    // use nl_langinfo() to retrieve language and locale info
    // in particular, nl_langinfo(CODSET) for character encoding
    // use localeconv() to retrieve just LC_NUMERIC and LC_MONETARY settings

    char *codeset = nl_langinfo(CODESET);
    if (strcmp(codeset, "UTF-8") != 0) {
        err("locale \"%s\": not a UTF-8 codeset: \"%s\"", loc, codeset);
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {

    const char *temp = "lstime";
    if (argv[0] != NULL) {
        temp = strrchr(argv[0], '/');          /* find last slash */
        if (temp == NULL || temp[1] == '\0') { /* if none or trailing slash */
            temp = argv[0];                    /* use fullpath */
        } else {
            temp++;                            /* use last part of name */
        }
    }
    lstime_set_prog(temp);
    if (!set_my_app_locale("")) {
        const char *fallback = "C.utf8";
        warn("trying a fallback locale: \"%s\"", fallback); 
        if (!set_my_app_locale(fallback)) {
            err("terminating: need a locale with a UTF-8 charset");
            exit(14);
        }
    }

    lstime_driver(stdout, argc, argv);

    return EXIT_SUCCESS;
}
