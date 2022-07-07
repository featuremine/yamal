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
 * @file time.cpp
 * @author Federico Ravchina
 * @date 28 Jun 2021
 * @brief File contains tests for FMC time API
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc/platform.h>
#include <fmc/time.h>

using namespace std;

#define ERROR_UNINITIALIZED_VALUE ((fmc_error_t *)1)

TEST(fmc, strptime) {
  struct tm t {};
  fmc_strptime("20210628-13:14:15.", "%Y%m%d-%H:%M:%S.", &t);
  EXPECT_EQ(t.tm_sec, 15);
  EXPECT_EQ(t.tm_min, 14);
  EXPECT_EQ(t.tm_hour, 13);
  EXPECT_EQ(t.tm_mday, 28);
  EXPECT_EQ(t.tm_mon, 5);
  EXPECT_EQ(t.tm_year, 121);
  EXPECT_EQ(t.tm_wday, 1);
  EXPECT_EQ(t.tm_yday, 178);
  EXPECT_EQ(t.tm_isdst, 0);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
