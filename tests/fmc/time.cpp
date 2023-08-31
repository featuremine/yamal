/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
  fmc_time64_t time = fmc_time64_from_nanos(10);
  fmc_time64_t time_expected = fmc_time64_from_nanos(10);
  int64_t timeraw = fmc_time64_raw(time);
  int64_t timeraw_expected = fmc_time64_raw(time_expected);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_from_raw(timeraw)), timeraw_expected);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_from_nanos(timeraw)), timeraw_expected);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_from_seconds(timeraw)),
            fmc_time64_raw(fmc_time64_from_nanos(timeraw * 1000000000ULL)));
  ASSERT_EQ(fmc_time64_to_nanos(time), timeraw_expected);
  ASSERT_EQ(fmc_time64_to_fseconds(fmc_time64_from_nanos(1000000000ULL)),
            (double)1.0);
  ASSERT_EQ(fmc_time64_raw(time), timeraw_expected);
}

TEST(fmc, comparisons) {
  fmc_time64_t time1 = fmc_time64_from_raw(1);
  fmc_time64_t time2 = fmc_time64_from_raw(2);
  ASSERT_TRUE(fmc_time64_less(time1, time2));
  ASSERT_FALSE(fmc_time64_greater(time1, time2));
  ASSERT_FALSE(fmc_time64_equal(time1, time2));
  ASSERT_FALSE(fmc_time64_is_end(time1));
  ASSERT_TRUE(fmc_time64_is_end(fmc_time64_end()));
}

TEST(fmc, operators) {
  fmc_time64_t time1 = fmc_time64_from_raw(10);
  fmc_time64_t time2 = fmc_time64_from_raw(20);
  ASSERT_EQ(fmc_time64_div(time2, time1), 2);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_add(time2, time1)), 30);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_sub(time2, time1)), 10);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_mul(time2, 10)), 200);
  ASSERT_EQ(fmc_time64_raw(fmc_time64_int_div(time2, 10)), 2);
  fmc_time64_inc(&time1, time2);
  ASSERT_EQ(fmc_time64_raw(time1), 30);
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
