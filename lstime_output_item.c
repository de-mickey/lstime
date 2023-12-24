// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lstime_private.h"

// higher level convenience function
void lstime_output_item(FILE *fp,
                        const lstime_info *info,
                        const lstime_options *opts) {
    lstime_out_it(fp,
                  info,
                  opts->item_format,
                  opts->time_format,
                  opts->format_time_as_utc,
                  opts->debug);
}

// lower level function, good for testing
void lstime_out_it(FILE *fp,
                   const lstime_info *info,
                   const char* item_format,
                   const char* time_format,
                   bool utc,
                   bool debug) {
    for (size_t i = 0 ; i < strlen(item_format) ; ++i) {
        int c = item_format[i];
        if (c == '%') {
            c = item_format[++i];
            switch (c) {
                case 'm':
                    fprintf(fp, "%s", lstime_format_timestamp(
                                info->mtime, time_format, utc));
                    break;
                case 'a':
                    fprintf(fp,"%s",  lstime_format_timestamp(
                                info->atime, time_format, utc));
                    break;
                case 'c':
                    fprintf(fp,"%s",  lstime_format_timestamp(
                                info->ctime, time_format, utc));
                    break;
                case 'b':
                    fprintf(fp,"%s",  lstime_format_timestamp(
                                info->btime, time_format, utc));
                    break;
                case 'n':
                    fputc('\n', fp);
                    break;
                case 'p':
                    fprintf(fp, "%s", lstime_format_path(info->path,
                                                         false, debug));
                    break;
                case 'r':
                    fprintf(fp, "%s", info->path);
                    break;
                case 'u':
                    fprintf(fp, "%s", lstime_format_path(info->path,
                                                         true, debug));
                    break;
                case 'z':
                    fputc('\0', fp);
                    break;
                case '%':
                    fputc('%', fp);
                    break;
                default:           // unrecognized % escape
                    err("unrecognized --output-format directive: %%%c",
                        (unsigned char) c);
                    exit(15);
                    break;
            }
        } else {
            fputc((unsigned char) c, fp);
        }
    }
}

