# SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
# SPDX-License-Identifier: GPL-3.0-or-later

SHELL = /bin/bash
CC = gcc
CFLAGS = \
    -std=c17 \
    -pedantic \
    -Wall \
    -Wextra \
    -Wmissing-prototypes \
    -Wshadow \
    -Wwrite-strings \
    -Werror \
    -D_POSIX_C_SOURCE=200809L \
    -D_XOPEN_SOURCE=700 \
    -D_FORTIFY_SOURCE=2 \
    -D_GNU_SOURCE \
    -O2

OBJS = \
    lstime_of_path.o \
    lstime_parse_options.o \
    lstime_iconv.o \
    lstime_stat_path.o \
    lstime_output_item.o \
    lstime_format_path.o \
    lstime_format_timestamp.o \
    lstime_sort_list.o \
    lstime_msg.o


all : lstime alltests

lstime : lstime.o $(OBJS)

lstime.o : lstime.h lstime_private.h

lstime_format_path.o : lstime.h lstime_private.h

lstime_format_timestamp.o : lstime.h lstime_private.h

lstime_iconv.o : lstime.h lstime_private.h

lstime_of_path.o : lstime.h lstime_private.h

lstime_output_item.o : lstime.h lstime_private.h

lstime_parse_options.o : lstime.h lstime_private.h

lstime_sort_list.o : lstime.h lstime_private.h

lstime_stat_path.o : lstime.h lstime_private.h

mymsg.o : lstime.h lstime_private.h


TESTOBJS = \
    lstime_tests.o \
    lstime_format_path_tests.o \
    lstime_format_timestamp_tests.o \
    lstime_output_item_tests.o \
    ddmunit.o

TESTPGM = lstime_tests

TESTOUT = lstime_tests.out

alltests : $(TESTOUT)

$(TESTOUT) : $(TESTPGM)
	set -o pipefail ; ./$< 2>&1 | tee $@

$(TESTPGM) : $(TESTOBJS) $(OBJS)

lstime_format_timestamp_tests.o : lstime_tests.h lstime.h lstime_private.h ddmunit.h

lstime_output_item_tests.o : lstime_tests.h lstime.h lstime_private.h ddmunit.h

lstime_format_path_tests.o : lstime_tests.h lstime.h ddmunit.h

lstime_tests.o : lstime_tests.h lstime.h ddmunit.h  

ddmunit.o : ddmunit.h


clean :
	$(RM) lstime lstime.o lstime_example lstime_example.o $(OBJS)
	$(RM) $(TESTOUT) $(TESTPGM) $(TESTOBJS)

.PHONY : all clean alltests

