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
 * @file common.cpp
 * @author Maxim Trokhimtchouk
 * @date 9 Aug 2021
 * @brief File contains tests for YTP yamal layer
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>
using namespace std;

TEST(yamal, common) {
  auto *error = (fmc_error_t *)nullptr;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);
  ASSERT_TRUE(fmc_fvalid(fd));
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
