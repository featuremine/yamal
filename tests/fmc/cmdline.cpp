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

int _argc;
const char **_argv;

const char *one = nullptr;
const char *two = nullptr;
const char *three = nullptr;

fmc_cmdline_opt_t options[] = {{"--help", false, nullptr},
                               {"--one", true, &one},
                               {"--two", true, &two},
                               {"--three", true, &three},
                               {nullptr}};

TEST(fmc_cmdline, values) {
  fmc_error_t *err = nullptr;

  fmc_cmdline_opt_proc(_argc, _argv, options, &err);
  ASSERT_EQ(err, nullptr);

  ASSERT_STREQ(one, "one");
  ASSERT_STREQ(two, "two");
  ASSERT_STREQ(three, "three");
}

TEST(fmc_cmdline, no_required) {
  fmc_error_t *err = nullptr;

  fmc_cmdline_opt_proc(_argc, _argv, options, &err);
  ASSERT_NE(err, nullptr);

  ASSERT_STREQ(fmc_error_msg(err),
               "option --one is required and remains unset");
}

TEST(fmc_cmdline, duplicated) {
  fmc_error_t *err = nullptr;

  fmc_cmdline_opt_proc(_argc, _argv, options, &err);
  ASSERT_NE(err, nullptr);

  ASSERT_STREQ(fmc_error_msg(err), "option --two is repeated");
}

TEST(fmc_cmdline, option) {
  fmc_error_t *err = nullptr;

  fmc_cmdline_opt_proc(_argc, _argv, options, &err);
  ASSERT_EQ(err, nullptr);

  ASSERT_TRUE(options[0].set);
  ASSERT_FALSE(options[1].set);
  ASSERT_FALSE(options[2].set);
  ASSERT_FALSE(options[3].set);
}

TEST(fmc_cmdline, extra) {
  fmc_error_t *err = nullptr;

  fmc_cmdline_opt_proc(_argc, _argv, options, &err);
  ASSERT_NE(err, nullptr);

  ASSERT_STREQ(fmc_error_msg(err),
               "option --help is given a value, but none expected");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  _argc = argc;
  _argv = (const char **)argv;

  return RUN_ALL_TESTS();
}
