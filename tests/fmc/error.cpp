/******************************************************************************

        COPYRIGHT (c) 2018 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file error.cpp
 * @author Federico Ravchina
 * @date 28 Jun 2021
 * @brief File contains tests for FMC error API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/error.h>

#include <fmc++/gtestwrap.hpp>
using namespace std;

TEST(error, multiple_errors) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_error_set(&err, "1");
  ASSERT_NE(err, nullptr);
  fmc_error_set(&err, "2");
  ASSERT_NE(err, nullptr);
  ASSERT_EQ(string_view(fmc_error_msg(err)), "2");
  fmc_error_destroy(err);
}

TEST(error, long_error) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  string long_string1(2048, '1');
  string medium_string1(700, '2');
  string medium_string2(700, '3');
  string expected_string1(2048, '1');
  string expected_string2(700, '2');
  string expected_string3(700, '3');
  fmc_error_set(&err, long_string1.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), expected_string1);
  fmc_error_set(&err, medium_string1.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), expected_string2);
  fmc_error_clear(&err);

  fmc_error_set(&err, medium_string1.c_str());
  fmc_error_set(&err, medium_string2.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), expected_string3);
  fmc_error_destroy(err);
  fmc_error_clear(&err);
}

TEST(error, append_1) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  string string1("1");
  string string1_append("2");
  string string1_expected("1, 2");

  // Append new string = set
  fmc_error_append(&err, string1.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1);
  fmc_error_append(&err, string1_append.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1_expected);

  fmc_error_destroy(err);
  fmc_error_clear(&err);
}

TEST(error, append_2) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  string string1("1");
  string string1_append("2");
  string string1_expected("1, 2");

  fmc_error_set(&err, string1.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1);
  fmc_error_append(&err, string1_append.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1_expected);

  fmc_error_destroy(err);
  fmc_error_clear(&err);
}

TEST(error, append_3) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  string string1("1");
  string string1_append("2");
  string string1_expected("1, 2");
  string string1_expected2("1");

  fmc_error_set(&err, string1.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1);
  fmc_error_append(&err, string1_append.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1_expected);
  fmc_error_set(&err, string1.c_str());
  ASSERT_EQ(string_view(fmc_error_msg(err)), string1_expected2);

  fmc_error_destroy(err);
  fmc_error_clear(&err);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
