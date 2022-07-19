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

TEST(fmc, conversions) {
  fm_time64_t time = 10;
  fm_time64_t time_expected = 10;
  int64_t timeraw = 10;
  int64_t timeraw_expected = 10;
  ASSERT_EQ(fm_time64_from_raw(timeraw), time_expected);
  ASSERT_EQ(fm_time64_from_nanos(timeraw), time_expected);
  ASSERT_EQ(fm_time64_from_seconds(timeraw),
            fm_time64_from_nanos(timeraw * 1000000000ULL));
  ASSERT_EQ(fm_time64_to_nanos(time), timeraw_expected);
  ASSERT_EQ(fm_time64_to_fseconds(time * 1000000000ULL),
            (double)timeraw_expected);
  ASSERT_EQ(fm_time64_raw(time), timeraw_expected);
}

TEST(fmc, comparisons) {
  fm_time64_t time1 = fm_time64_from_raw(1);
  fm_time64_t time2 = fm_time64_from_raw(2);
  ASSERT_TRUE(fm_time64_less(time1, time2));
  ASSERT_FALSE(fm_time64_greater(time1, time2));
  ASSERT_FALSE(fm_time64_equal(time1, time2));
  ASSERT_FALSE(fm_time64_is_end(time1));
  ASSERT_TRUE(fm_time64_is_end(fm_time64_end()));
}

TEST(fmc, operators) {
  fm_time64_t time1 = fm_time64_from_raw(10);
  fm_time64_t time2 = fm_time64_from_raw(20);
  ASSERT_EQ(fm_time64_div(time2, time1), 2);
  ASSERT_EQ(fm_time64_add(time2, time1), fm_time64_from_raw(30));
  ASSERT_EQ(fm_time64_sub(time2, time1), fm_time64_from_raw(10));
  ASSERT_EQ(fm_time64_mul(time2, 10), fm_time64_from_raw(200));
  ASSERT_EQ(fm_time64_int_div(time2, 10), fm_time64_from_raw(2));
  fm_time64_inc(&time1, time2);
  ASSERT_EQ(fm_time64_raw(time1), 30);
}

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
