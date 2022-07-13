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
 * @file extension.cpp
 * @date 13 Jul 2022
 * @brief File contains tests for extensions
 *
 * @see http://www.featuremine.com
 */

#include <fmc/error.h>
#include <fmc/extension.h>

#include <fmc++/gtestwrap.hpp>


std::string libfmc_shared_path;

TEST(error, extension_simple) {
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_ext_t ext = fmc_ext_open(libfmc_shared_path.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(ext, nullptr);
  fmc_ext_close(ext);
}

TEST(error, extension_symbol2) {
  typedef long long (*fmc_llround_t)(double);
  fmc_llround_t llround;
  fmc_error_t *err;
  fmc_error_clear(&err);
  ASSERT_EQ(err, nullptr);
  fmc_ext_t ext = fmc_ext_open(libfmc_shared_path.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(ext, nullptr);
  llround = (fmc_llround_t)fmc_ext_sym(ext, "fmc_llround", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_NE(llround, nullptr);
  long long ret = llround(555.88);
  ASSERT_EQ(ret, 556);
  fmc_ext_close(ext);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  libfmc_shared_path = argv[1];
  return RUN_ALL_TESTS();
}