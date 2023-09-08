/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file cmdline.cpp
 * @author Maxim Trokhimtchouk
 * @date 7 Aug 2023
 * @brief File contains tests for cmdline API
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc/cmdline.h>

using namespace std;

const char *one = nullptr;
const char *two = nullptr;
const char *three = nullptr;

TEST(fmc_cmdline, fmc_cmdline_opt_proc) {
    ASSERT_EQ(one, "one");
    ASSERT_EQ(two, "two");
    ASSERT_EQ(three, "three");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

    fmc_cmdline_opt_t options[] = {
        {"--one", true, &one},
        {"--two", true, &two},
        {"--three", true, &three},
        {NULL}
    };

    if (!fmc_cmdline_opt_proc(argc, argv, options))
        return 1;

  return RUN_ALL_TESTS();
}
