/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file cmdline.c
 * @date 7 Aug 2023
 * @brief Yet another command line arguments parser API
 *
 * Yet another command line arguments parser API.
 * @see http://www.featuremine.com
 */

#include <fmc/cmdline.h>
#include <fmc/error.h>
#include <string.h> // strlen()


const char *fmc_cmdline_opt(int argc, const char **argv, const char *opt)
{
	size_t n = strlen(opt);
	int c = argc;

	while (--c > 0) {

		if (!strncmp(argv[c], opt, n)) {
			if (!*(argv[c] + n) && c < argc - 1) {
				if (!argv[c + 1] || strlen(argv[c + 1]) > 1024)
					return NULL;
				return argv[c + 1];
			}

			if (argv[c][n] == '=')
				return &argv[c][n + 1];
			return argv[c] + n;
		}
	}

	return NULL;
}

void fmc_cmdline_opt_proc(int argc, const char **argv,
                          fmc_cmdline_opt_t *opts, fmc_error_t **err)
{
	const char *p;
    int n;

    fmc_error_clear(err);

	for (n = 0; opts[n].str; ++n) {
		opts[n].set = false;
		p = fmc_cmdline_opt(argc, argv, opts[n].str);
		if (!p)
			continue;
		if (opts[n].set) {
            fmc_error_set(err, "option %s is repeated (%s:%d)", opts[n].str, __FILE__, __LINE__);
			return;
		}
		opts[n].set = true;
		*opts[n].value = p;
	}
	for (n = 0; opts[n].str; ++n) {
		if (!opts[n].required || opts[n].set)
			continue;
        fmc_error_add(err, "\n", "option %s is required and remains"
                      " unset (%s:%d)", opts[n].str, __FILE__, __LINE__);
	}
	return;
}