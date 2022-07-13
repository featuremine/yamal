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

TEST(error, multiple_errors) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_error_set(&err, "1");
  ASSERT_NE(err, nullptr);
  fmc_error_set(&err, "2");
  ASSERT_NE(err, nullptr);
  ASSERT_EQ(std::string_view(fmc_error_msg(err)), "2");
  fmc_error_destroy(err);
}

TEST(error, long_error) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  std::string long_string1(2048, '1');
  std::string medium_string1(700, '2');
  std::string medium_string2(700, '3');
  std::string expected_string1(2048, '1');
  std::string expected_string2(700, '2');
  std::string expected_string3(700, '3');
  fmc_error_set(&err, long_string1.c_str());
  ASSERT_EQ(std::string_view(fmc_error_msg(err)), expected_string1);
  fmc_error_set(&err, medium_string1.c_str());
  ASSERT_EQ(std::string_view(fmc_error_msg(err)), expected_string2);
  fmc_error_clear(&err);

  fmc_error_set(&err, medium_string1.c_str());
  fmc_error_set(&err, medium_string2.c_str());
  ASSERT_EQ(std::string_view(fmc_error_msg(err)), expected_string3);
  fmc_error_destroy(err);
  fmc_error_clear(&err);
}

TEST(error, init_none) {
  fmc_error_t err;
  std::string errormsg("None");
  fmc_error_init_none(&err);

  ASSERT_EQ(err.code, FMC_ERROR_NONE);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err)), errormsg);

  fmc_error_destroy(&err);
}

TEST(error, init_custom) {
  fmc_error_t err;
  std::string errormsg("custom error message");
  fmc_error_init(&err, FMC_ERROR_CUSTOM, errormsg.c_str());

  ASSERT_EQ(err.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err)), errormsg);

  fmc_error_destroy(&err);
}

TEST(error, init_custom_unknown) {
  fmc_error_t err;
  std::string errormsg("UNKNOWN");
  fmc_error_init(&err, FMC_ERROR_CUSTOM, NULL);

  ASSERT_EQ(err.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err)), errormsg);

  fmc_error_destroy(&err);
}

TEST(error, init_memory) {
  fmc_error_t err;
  std::string errormsg("Could not allocate memory");
  fmc_error_init(&err, FMC_ERROR_MEMORY, nullptr);

  ASSERT_EQ(err.code, FMC_ERROR_MEMORY);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err)), errormsg);

  fmc_error_destroy(&err);
}

TEST(error, init_sprintf) {
  fmc_error_t err;
  std::string errormsg("custom error message 1");
  fmc_error_init_sprintf(&err, "%s %d", "custom error message", 1);

  ASSERT_EQ(err.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err)), errormsg);

  fmc_error_destroy(&err);
}

TEST(error, copy) {
  fmc_error_t err1;
  std::string errormsg1("Could not allocate memory");
  fmc_error_init(&err1, FMC_ERROR_MEMORY, nullptr);
  ASSERT_EQ(err1.code, FMC_ERROR_MEMORY);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err1)), errormsg1);

  fmc_error_t err2;
  std::string errormsg2("custom error message");
  fmc_error_init(&err2, FMC_ERROR_CUSTOM, errormsg2.c_str());
  ASSERT_EQ(err2.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err2)), errormsg2);

  fmc_error_cpy(&err1, &err2);
  ASSERT_EQ(err1.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err1)), errormsg2);

  fmc_error_destroy(&err1);
  fmc_error_destroy(&err2);
}

TEST(error, join) {
  fmc_error_t err1;
  std::string errormsg1("Could not allocate memory");
  fmc_error_init(&err1, FMC_ERROR_MEMORY, nullptr);
  ASSERT_EQ(err1.code, FMC_ERROR_MEMORY);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err1)), errormsg1);

  fmc_error_t err2;
  std::string errormsg2("custom error message");
  fmc_error_init(&err2, FMC_ERROR_CUSTOM, errormsg2.c_str());
  ASSERT_EQ(err2.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err2)), errormsg2);

  fmc_error_t err3;
  fmc_error_init_join(&err3, &err1, &err2, "; ");
  ASSERT_EQ(err3.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err3)), errormsg1 + "; " + errormsg2);

  fmc_error_destroy(&err1);
  fmc_error_destroy(&err2);
  fmc_error_destroy(&err3);
}

TEST(error, cat) {
  fmc_error_t err1;
  std::string errormsg1("Could not allocate memory");
  fmc_error_init(&err1, FMC_ERROR_MEMORY, nullptr);
  ASSERT_EQ(err1.code, FMC_ERROR_MEMORY);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err1)), errormsg1);

  fmc_error_t err2;
  std::string errormsg2("custom error message");
  fmc_error_init(&err2, FMC_ERROR_CUSTOM, errormsg2.c_str());
  ASSERT_EQ(err2.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err2)), errormsg2);

  fmc_error_cat(&err1, &err2, ", ");
  ASSERT_EQ(err1.code, FMC_ERROR_CUSTOM);
  ASSERT_EQ(std::string_view(fmc_error_msg(&err1)), errormsg1 + ", " + errormsg2);

  fmc_error_destroy(&err1);
  fmc_error_destroy(&err2);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
