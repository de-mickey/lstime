// SPDX-FileCopyrightText: © 2023 Daniel D. Mickey III
// SPDX-License-Identifier: GPL-3.0-or-later

#include <time.h>

#include "lstime_tests.h"

du_state_t *du_global = NULL;

static void timezone_setup(const char *tz) {
    setenv("TZ", tz, 1);
    tzset();
}

int main() {
    // set a consistent, unusual, hard-coded TZ for reproducible tests
    timezone_setup("UTC+02:00");  // western Greenland, with no DST
    du_global = du_alloc_state();
    // du_show_success(true);

    format_path_suite();
    format_timestamp_suite();
    output_item_suite();
    int rc = du_total_summary(NULL);
    exit(rc);
}
