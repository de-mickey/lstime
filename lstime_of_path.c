// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <locale.h>
#include <langinfo.h>
#include <unistd.h>

#include "lstime_private.h"


static void cleanup(arr_wrapper *list) {
    lstime_iconv_finit();
    lstime_info *ptr = list->arr;
    lstime_info *end = ptr + list->num_elems;
    for ( ; ptr < end ; ++ptr) {
        free((void *)ptr->path); // override const
        ptr->path = NULL;
        if (ptr->sortkey != NULL) {
            free((void *)ptr->sortkey); // override const
            ptr->sortkey = NULL;
        }
    }
    free(list->arr);
    list->arr = NULL;
    list->capacity = 0;
    list->num_elems = 0;
}

void lstime_output_list(FILE *fpout,
                        const arr_wrapper *list,
                        const lstime_options *opts) {
    for (size_t i = 0; i < list->num_elems; ++i) {
        lstime_output_item(fpout, &list->arr[i], opts);
    }
}

void add_info_to_list(arr_wrapper *list, const lstime_info *info) {
    if (list->num_elems + 1 > list->capacity) {
        size_t new_cap = list->capacity * 2;
        if (new_cap < 2048) {
            new_cap = 2048;
        }
        list->arr = reallocarray(list->arr, new_cap, sizeof(lstime_info));
        if (list->arr == NULL) {
            err("list array out of memory: %s", strerror(errno));
            exit(32);
        }
        list->capacity = new_cap;
    }
    list->arr[list->num_elems] = *info;
    ++list->num_elems;
}

void lstime_of_path(FILE *fpout,
                    arr_wrapper *list,
                    const lstime_options *opts,
                    const char *path) {
    lstime_info info;
    info.path = strdup(path);
    info.sortkey = NULL;
    if (info.path == NULL) {
        err("strdup out of memory: %s", strerror(errno));
        exit(33);
    }
    if (lstime_stat_path(&info, opts->stat_flags) != 0) {
        err("lstime_stat_path: %s: %s", info.path, strerror(errno));
        exit(3);
    }

    if (opts->sort_field == 'n' || list == NULL) {  // sort=none, so immediately output
        lstime_output_item(fpout, &info, opts);
    } else {
        // build list for later sorting
        add_info_to_list(list, &info);
    }
}

void lstime_parse_path_input_file(FILE *fpout,
                                  arr_wrapper *list,
                                  const lstime_options *opts,
                                  const char *infile) {
    FILE *fpin = NULL;
    if (strcmp(infile, "-") == 0) {
        fpin = stdin;
    } else {
        if ((fpin = fopen(infile, "r")) == NULL) {
            err("fopen: %s: %s", infile, strerror(errno));
            exit(12);
        }
    }

    char *path = NULL;
    size_t capacity = 0;
    ssize_t rc = 0;
    while ((rc = getdelim(&path, &capacity, opts->path_input_file_delim, fpin)) != -1) {
        if (opts->path_input_file_delim == '\n' && path[rc - 1] == '\n') {
            path[rc - 1] = '\0';  // trim trailing newline
        }
        lstime_of_path(fpout, list, opts, path);
    }
    if (!feof(fpin)) { // there must have been an IO error
        err("getdelim: %s: IO error: %s", infile, strerror(errno));
        exit(4);
    }

    if (fpin != stdin) {
        fclose(fpin);
    }
    free(path);
}

void lstime_driver(FILE *fpout, int argc, char *argv[]) {
    lstime_options opts;
    lstime_set_option_defaults(&opts);
    lstime_parse_options(&opts, argc, argv);
    arr_wrapper list;
    memset(&list, 0, sizeof(list));

    if (opts.path_input_file != NULL && opts.path_input_file[0] != '\0') {
        lstime_parse_path_input_file(fpout, &list, &opts, opts.path_input_file);
    }
    for (; optind < argc; optind++) {
        char *path = argv[optind];
        lstime_of_path(fpout, &list, &opts, path);
    }
    lstime_sort_list(&list, &opts);
    lstime_output_list(fpout, &list, &opts);

    cleanup(&list);  // about to exit, so this cleanup is optional
}
