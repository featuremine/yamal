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

static bool fmc_cmdline_opt_parse(int argc, const char **argv, const char *opt,
                                  const char **val, fmc_error_t **err) {
  size_t n = strlen(opt);
  int c = argc;
  bool found = false;

  fmc_error_clear(err);

  while (--c > 0) {
    if (!strncmp(argv[c], opt, n)) {
      if (found) {
        fmc_error_set(err, "option %s is repeated", opt);
        return false;
      }
      found = true;
      if (!val) {
        if (*(argv[c] + n)) {
          fmc_error_set(err, "option %s is given a value, but none expected", opt);
          return false;
        }
        continue;
      }
      if (!*(argv[c] + n) && c < argc - 1) {
        *val = argv[c + 1];
        continue;
      }
      *val = argv[c][n] == '=' ? &argv[c][n + 1] : argv[c] + n;
    }
  }

  return found;
}

void fmc_cmdline_opt_proc(int argc, const char **argv, fmc_cmdline_opt_t *opts,
                          fmc_error_t **err) {
  fmc_error_clear(err);

  for (int n = 0; opts[n].str; ++n) {
    opts[n].set =
        fmc_cmdline_opt_parse(argc, argv, opts[n].str, opts[n].value, err);
    if (*err)
      return;
  }
  for (int n = 0; opts[n].str; ++n) {
    if (!opts[n].required || opts[n].set)
      continue;
    fmc_error_add(err, "\n",
                  "option %s is required and remains unset", opts[n].str);
  }
  return;
}
