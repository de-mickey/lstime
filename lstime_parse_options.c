// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include "lstime_private.h"

static const char *usage_fmt = "\nUsage:  %s [options] [path ...]\n%s%s";
static const char *usage1 =
"\n"
"Description:  Display a file's associated timestamps.\n"
"\n"
"Options:\n"
"  -i, --item-format={ifmt}  item (overall) format\n"
"  -t, --time-format={tfmt}  strftime format for timestamps\n"
"  -l, --local-time          use local (TZ) timezone (default)\n"
"  -u, --utc                 use UTC/GMT/Z timezone\n"
"  -f, --file={filename}     read pathnames from file (use - for stdin)\n"
"  -n, --newline             read paths with newline termination (default)\n"
"  -z, --null                read paths with nul-termination\n"
"  -o, --show-options        show option settings (including defaults)\n"
"  -r, --reverse             reverse sorting order\n"
"  -s, --sort={field}        sort output by field (default is newest first)\n"
"  -d, --debug               show some debug messages\n"
"  -v, --version             show version info\n"
"  -h, --help                show this usage help\n"
"\n"
"   The item format {ifmt} specifies which timestamp fields to output,\n"
"   their order, and the surrounding context.  The currently supported\n"
"   format specifiers are:\n"
"      %m    mtime, last modification timestamp\n"
"      %a    atime, last access timestamp\n"
"      %c    ctime, last change of metadata (inode) timestamp\n"
"      %b    btime, birth (creation) timestamp\n"
"      %r    raw item pathname (raw OS bytes)\n"
"      %p    item pathname (includes escapes for unusual characters)\n"
"      %u    item pathname (also has escapes for codepoints)\n"
"      %n    newline\n"
"      %z    zero-byte, nul character\n"
"      %%    literal percent sign\n"
"      (anything else is literal output)\n"
"      The default is:  --output-format='%m  %a  %p%n'\n"
"\n"
"   The time format {tfmt} is specified by strftime(3), with two extensions;\n"
"      A '%[1-9]N' specifier will format 1 to 9 digits of subsecond time.\n"
"         %3N     3 digits (milliseconds)\n"
"         %6N     6 digits (microseconds)\n"
"         %N      defaults to 9 digits (nanoseconds)\n"
"      A '%:z' specifier will format timezone offset as '[+-]HH:MM'\n"
"         for example '-05:00'\n"
"         The '%:z' format (UTC offset with colon) is used by RFC 3339.\n"
"      The default tfmt is:  --time-format='%FT%T.%3N'\n"
"\n";

static const char *usage2 =
"   The sort field value is one of:\n"
"      m[time] | a[time] | c[time] | b[time] | p[ath] | n[one] (default)\n"
"      Times are by default sorted most recent first.\n"
"      Sort order can be reversed with -r/--reverse.\n"
"      Note that sorting buffers all output, which can use lots of memory.\n"
"      But '--sort none' will not buffer and is preferred when processing\n"
"      large inputs (like from find).\n"
"      Sorting by path uses the raw path and the current locale's collation.\n"
"      Set LC_COLLATE=C to ignore the locale's collation.\n"
"\n"
"   The following are just convenience presets for some -i/-t settings:\n"
"   -m, --mtime         mtime only\n"
"   -a, --atime         atime only\n"
"   -c, --ctime         ctime only\n"
"   -b, --btime         btime only\n"
"   -e, --everything    expanded multiline format with labels\n"
"\n"
"   The following more advanced options are not usually necessary\n"
"   (man statx(2) for details):\n"
"   -A, --automount     (statx only) allow automounting of filesystems (default)\n"
"   -B, --no-automount  (statx only) avoid automounting, use underlying directory\n"
"   -L, --follow-links  follow symlinks to target timestamps (default)\n"
"   -P, --stat-links    show timestamps of the symlink itself\n"
"   -X, --sync-as-stat  (statx only) sync as does stat(2) (default)\n"
"   -Y, --force-sync    (statx only) force syncing of attributes from remote fs\n"
"   -Z, --do-not-sync   (statx only) avoid syncing of remote fs, use cache\n"
"\n"
"   Note that -L and -P only apply to final path components (basenames)\n"
"   that are symlinks. Symlinks earlier in a path are always followed.\n"
"\n"
"   To read input paths that are nul-terminated, use the -z/--null option.\n"
"   To write output paths that are nul-terminated, use a -i/--item-format\n"
"   with %z after the raw path, e.g., -i '%m %r%z'.\n"
"   Paths with escaping (%p and %u) do not really need nul-termination.\n"
"\n"
"   To use a timezone (other than local (-l) or UTC (-u), set the\n" 
"   TZ environment variable. Note that TZ uses UTC offsets with the\n"
"   sign reversed from ISO 8601 offsets. For example, Eastern Standard\n"
"   Time is equivalent to: TZ='UTC+05:00' or TZ='Etc/GMT+5'\n"
"\n"
"Caveats:\n"
"   Uses statx(2) system call which is specific to recent Linux kernels.\n"
"   Can be recompiled to use stat(2) and lstat(2) instead.\n"
"   Some file system types do not preserve/provide btime timestamps.\n"
"   Missing timestamps will display as 'N/A'.\n"
"\n"
"Notes:\n"
"   ctime is updated by:\n"
"      file modifications    (because mtime changes)\n"
"      chmod,chown,chgrp     (metadata changes)\n"
"      mv,ln,rename changes  (reference count changes)\n"
"   So ctime is normally equal or newer than mtime.\n"
"   Changes to atime alone do not update ctime.\n"
"   ctime and btime cannot be directly manipulated\n"
"   (short of playing with system time, fudging disk images, etc).\n"
"   Editors and other tools often write their updates to a new file,\n"
"   then rename it back to original, thus losing the original btime.\n"
"   When (or if) atime gets updated depends upon fs mount options.\n"
"\n";

static const char *short_opts = "+:abcdef:hi:lmnors:t:uvzABLPXYZ";

static struct option long_opts[] = {
    { "atime",            no_argument,       NULL, 'a'},
    { "btime",            no_argument,       NULL, 'b'},
    { "ctime",            no_argument,       NULL, 'c'},
    { "debug",            no_argument,       NULL, 'd'},
    { "everything",       no_argument,       NULL, 'e'},
    { "file",             required_argument, NULL, 'f'},
    { "help",             no_argument,       NULL, 'h'},
    { "item-format",      required_argument, NULL, 'i'},
    { "local-time",       no_argument,       NULL, 'l'},
    { "mtime",            no_argument,       NULL, 'm'},
    { "newline",          no_argument,       NULL, 'n'},
    { "show-options",     no_argument,       NULL, 'o'},
    { "reverse",          no_argument,       NULL, 'r'},
    { "sort",             required_argument, NULL, 's'},
    { "time-format",      required_argument, NULL, 't'},
    { "utc",              no_argument,       NULL, 'u'},
    { "version",          no_argument,       NULL, 'v'},
    { "null",             no_argument,       NULL, 'z'},
    { "automount",        no_argument,       NULL, 'A'},
    { "no-automount",     no_argument,       NULL, 'B'},
    { "follow-links",     no_argument,       NULL, 'L'},
    { "stat-links",       no_argument,       NULL, 'P'},
    { "sync-as-stat",     no_argument,       NULL, 'X'},
    { "force-sync",       no_argument,       NULL, 'Y'},
    { "do-not-sync",      no_argument,       NULL, 'Z'},
    { NULL, 0, NULL, 0 }
};

static const char *long_from_short(int short_opt) {
    for (struct option *ptr = long_opts ; ptr->name != NULL ; ++ptr) {
        if (ptr->val == short_opt) {
            return ptr->name;
        }
    }
    return "";
}

void lstime_set_option_defaults(lstime_options *opts) {
    opts->item_format = "%m  %a  %p%n";
    opts->time_format = "%FT%T.%3N";
    opts->path_input_file = NULL;
    opts->stat_flags = AT_STATX_SYNC_AS_STAT; // also defaults to follow, automount
    opts->sort_field = 'n';
    opts->reverse = false;
    opts->path_input_file_delim = '\n';
    opts->format_time_as_utc = false;
    opts->debug = false;
}

void lstime_show_option_settings(const lstime_options *opts, FILE *fp) {
    fprintf(fp, "--item-format=\"%s\"\n", opts->item_format);
    fprintf(fp, "--time-format=\"%s\"\n", opts->time_format);
    if (opts->path_input_file != NULL && opts->path_input_file[0] != '\0') {
        fprintf(fp, "--file=\"%s\"\n", opts->path_input_file);
    }

    fprintf(fp, (opts->stat_flags & AT_SYMLINK_NOFOLLOW) ?
            "--stat-links\n" : "--follow-links\n");
    fprintf(fp, (opts->stat_flags & AT_NO_AUTOMOUNT) ?
            "--no-automount\n" : "--automount\n");
    // SYNC_AS_STAT is signaled by both FORCE_SYNC and DONT_SYNC being unset
    if ((opts->stat_flags & AT_STATX_SYNC_TYPE) == AT_STATX_SYNC_AS_STAT) {
        fprintf(fp, "--sync-as-stat\n");
    }
    if ((opts->stat_flags & AT_STATX_SYNC_TYPE) == AT_STATX_FORCE_SYNC) {
        fprintf(fp, "--force-sync\n");
    }
    if ((opts->stat_flags & AT_STATX_SYNC_TYPE) == AT_STATX_DONT_SYNC) {
        fprintf(fp, "--do-not-sync\n");
    }

    fprintf(fp, "--sort=%c\n", opts->sort_field);
    if (opts->reverse) {
        fprintf(fp, "--reverse\n");
    }
    if (opts->path_input_file_delim == '\n') {
        fprintf(fp, "--newline\n");
    } else if (opts->path_input_file_delim == '\0') {
        fprintf(fp, "--null\n");
    }
    fprintf(fp, "%s\n", (opts->format_time_as_utc) ? "--utc" : "--local-time");
    if (opts->debug) {
        fprintf(fp, "%s\n", "--debug");
    }
    fprintf(fp, "\n");
}

void lstime_parse_options(lstime_options *opts, int argc, char *argv[]) {
    int long_opt_index = 0;
    int opt = 0;
    const char *pgm = lstime_get_prog();

    while ((opt = getopt_long(argc, argv, short_opts,
                              long_opts, &long_opt_index)) != -1) {
        switch(opt) {
        case 'a':   //  --atime
            opts->item_format = "%a  %p%n";
            break;
        case 'b':   //  --btime
            opts->item_format = "%b  %p%n";
            break;
        case 'c':   //  --ctime
            opts->item_format = "%c  %p%n";
            break;
        case 'd':   //  --debug
            opts->debug = true;
            break;
        case 'e':   //  --everything
            opts->item_format = "%p\n"
                "    modified  %m\n"
                "    accessed  %a\n"
                "     changed  %c\n"
                "        born  %b\n"
                "\n";
            opts->time_format = "%F %T.%9N %:z";
            break;
        case 'f':   //  --file
            opts->path_input_file = optarg;
            break;
        case 'i':   //  --item-format
            opts->item_format = optarg;
            break;
        case 'l':   //  --local-time
            opts->format_time_as_utc = false;
            break;
        case 'm':   //  --mtime
            opts->item_format = "%m  %p%n";
            break;
        case 'n':   //  --newline
            opts->path_input_file_delim = '\n';
            break;
        case 'o':   //  --show-options
            lstime_show_option_settings(opts, stderr);
            break;
        case 'r':   //  --reverse
            opts->reverse = !opts->reverse;  // flips/toggles 
            break;
        case 's':   //  --sort
            if (strcmp(optarg, "m") == 0 ||
                strcmp(optarg, "mtime") == 0) {
                opts->sort_field = 'm';
            } else if (strcmp(optarg, "a") == 0 ||
                       strcmp(optarg, "atime") == 0) {
                opts->sort_field = 'a';
            } else if (strcmp(optarg, "c") == 0 ||
                       strcmp(optarg, "ctime") == 0) {
                opts->sort_field = 'c';
            } else if (strcmp(optarg, "b") == 0 ||
                       strcmp(optarg, "btime") == 0) {
                opts->sort_field = 'b';
            } else if (strcmp(optarg, "p") == 0 ||
                       strcmp(optarg, "path") == 0) {
                opts->sort_field = 'p';
            } else if (strcmp(optarg, "n") == 0 ||
                       strcmp(optarg, "none") == 0) {
                opts->sort_field = 'n';
            } else {
                err("unknown --sort value: %s\n", optarg);
                exit(2);
            }
            break;
        case 't':   //  --time-format
            opts->time_format = optarg;
            break;
        case 'u':   //  --utc
            opts->format_time_as_utc = true;
            break;
        case 'v':   //  --version
            printf("%s version 1.0.1\n", lstime_get_prog());
            printf("License GPL-3.0-or-later\n");
            printf("Copyright 2023 Daniel D. Mickey III\n\n");
            break;
        case 'z':   //  --null
            opts->path_input_file_delim = '\0';
            break;
        case 'A':   //  --automount
            opts->stat_flags &= ~AT_NO_AUTOMOUNT;
            break;
        case 'B':   //  --no-automount
            opts->stat_flags |= AT_NO_AUTOMOUNT;
            break;
        case 'L':   //  --follow-links
            opts->stat_flags &= ~AT_SYMLINK_NOFOLLOW;
            break;
        case 'P':   //  --stat-links
            opts->stat_flags |= AT_SYMLINK_NOFOLLOW;
            break;
        case 'X':   //  --sync-as-stat
            opts->stat_flags &= ~AT_STATX_SYNC_TYPE;
            opts->stat_flags |= AT_STATX_SYNC_AS_STAT;
            break;
        case 'Y':   //  --force-sync
            opts->stat_flags &= ~AT_STATX_SYNC_TYPE;
            opts->stat_flags |= AT_STATX_FORCE_SYNC;
            break;
        case 'Z':   //  --do-not-sync
            opts->stat_flags &= ~AT_STATX_SYNC_TYPE;
            opts->stat_flags |= AT_STATX_DONT_SYNC;
            break;
        case 'h':   //  --help
            fprintf(stdout, usage_fmt, pgm, usage1, usage2);
            exit(0);
            break;
        case ':':
            err("missing required option argument for: -%c/--%s",
                optopt, long_from_short(optopt));
            exit(1);
            break;
        case '?':
        case 0:
        default:
            // err("unknown or ambiguous option: opt=%d(%c)  optopt=%d(%c)\n",
            //    opt, opt, optopt, optopt);
            err("unknown or ambiguous argument: %s", argv[optind - 1]);
            exit(2);
            break;
        }
    }
}

