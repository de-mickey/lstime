// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LSTIME_H
#define LSTIME_H 1

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <iconv.h>
#include <time.h>

typedef struct timespec timespec;

typedef struct lstime_info {
    const char *path;
    const char *sortkey; // only used for sorting by path
    timespec mtime;
    timespec atime;
    timespec ctime;
    timespec btime;
} lstime_info;

typedef struct lstime_options {
    const char *item_format;
    const char *time_format;
    const char *path_input_file;
    int stat_flags;
    int path_input_file_delim;
    int sort_field;
    bool reverse;
    bool format_time_as_utc;
    bool debug;
} lstime_options;

typedef struct arr_wrapper {
    lstime_info *arr;
    size_t capacity;
    size_t num_elems;
} arr_wrapper;


void lstime_of_path(FILE *fpout,
                    arr_wrapper *list,
                    const lstime_options *opts,
                    const char *path);
void lstime_parse_path_input_file(FILE *fpout,
                                  arr_wrapper *list,
                                  const lstime_options *opts,
                                  const char *path);
void lstime_set_option_defaults(lstime_options *opts);
void lstime_parse_options(lstime_options *opts, int argc, char *argv[]);
void lstime_show_option_settings(const lstime_options *opts, FILE *fp);
int lstime_stat_path(lstime_info *info, int stat_flags);
void lstime_sort_list(arr_wrapper *list, const lstime_options *opts);
void lstime_output_list(FILE *fpout,
                        const arr_wrapper *list,
                        const lstime_options *opts);
const char *lstime_format_path(const char *path, bool escape_uni, bool debug);
const char *lstime_format_timestamp(const timespec ts,
                                    const char *time_format,
                                    bool format_time_as_utc);
void lstime_output_item(FILE *fp,
                        const lstime_info *info,
                        const lstime_options *opts);
void lstime_out_it(FILE *fp,
                   const lstime_info *info,
                   const char* item_format,
                   const char *time_format,
                   bool format_time_as_utc,
                   bool debug);
void add_info_to_list(arr_wrapper *list, const lstime_info *info);
void lstime_driver(FILE *fpout, int argc, char *argv[]);

#endif
