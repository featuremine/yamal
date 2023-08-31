/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file test.h
 * @author Maxim Trokhimtchouk
 * @date 11 Aug 2017
 * @brief Test utilities
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compares two files using diff
 *
 * @param base base filename
 * @param test test filename
 * @return true if files are the same, false otherwise
 */
FMMODFUNC bool fmc_run_base_vs_test_diff(const char *base, const char *test);

/**
 * @brief Compares two files using numdiff
 *
 * @param base base filename
 * @param test test filename
 * @return true if files are the same, false otherwise
 */
FMMODFUNC bool fmc_numdiff_base_vs_test(const char *base, const char *test);

#ifdef __cplusplus
}
#endif

#define ASSERT_BASE(base, test)                                                \
  ASSERT_TRUE(fmc_run_base_vs_test_diff(base, test))

#define ASSERT_NUMDIFF_BASE(base, test)                                        \
  ASSERT_TRUE(fmc_numdiff_base_vs_test(base, test))

#define EXPECT_BASE(base, test)                                                \
  EXPECT_TRUE(fmc_run_base_vs_test_diff(base, test))

#define EXPECT_NUMDIFF_BASE(base, test)                                        \
  EXPECT_TRUE(fmc_numdiff_base_vs_test(base, test))
