/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file math.cpp
 * @author Federico Ravchina
 * @date 28 Jun 2021
 * @brief File contains tests for FMC math API
 *
 * @see http://www.featuremine.com
 */

#define FMC_COUNTER_ENABLE

#include <fmc++/counters.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/alignment.h>
#include <fmc/math.h>

using namespace std;

TEST(fmc_math, FMC_FLOORLOG2) {
  for (uint64_t i = 0; i < 63; ++i) {
    uint64_t x = 1ULL << i;
    ASSERT_EQ(FMC_FLOORLOG2(x), i);
  }
  for (uint64_t i = 1; i < 63; ++i) {
    uint64_t x = (1UL << i) + 1;
    ASSERT_EQ(FMC_FLOORLOG2(x), i);
  }
}

TEST(fmc_math, floor_log10) {
  using namespace fmc::counter;
  auto res = floor_log10(0);
  ASSERT_EQ(res.first, 1);
  ASSERT_EQ(res.second, 0);
  for (uint64_t x = 1; x < 10; ++x) {
    res = floor_log10(x);
    ASSERT_EQ(res.first, 1);
    ASSERT_EQ(res.second, 1);
  }
  for (uint64_t i = 1; i < 10; ++i) {
    for (uint64_t j = 0; j < 10; ++j) {
      uint64_t x = i * 10ULL + j;
      res = floor_log10(x);
      ASSERT_EQ(res.first, 10);
      ASSERT_EQ(res.second, 2);
    }
  }
  for (uint64_t pow10 = 1, c = 1; c <= 17; ++c, pow10 *= 10ULL) {
    for (uint64_t i = 1; i < 10; ++i) {
      for (uint64_t j = 0; j < 100; ++j) {
        uint64_t x = (i * 100ULL + j) * pow10;
        {
          FMC_NANO_AVG(floor_log10);
          res = floor_log10(x);
        }
        res = floor_log10(x);
        ASSERT_EQ(res.first, pow10 * 100ULL);
        ASSERT_EQ(res.second, c + 2);
        ASSERT_LE(res.first, x);
        ASSERT_LT(x, res.first * 10UL);
      }
    }
  }
}

TEST(fmc_math, llround) {
  ASSERT_EQ(fmc_llround(0.0), 0ll);

  ASSERT_EQ(fmc_llround(1.0), 1ll);
  ASSERT_EQ(fmc_llround(-1.0), -1ll);

  ASSERT_EQ(fmc_llround(0.1), 0ll);
  ASSERT_EQ(fmc_llround(-0.1), 0ll);
  ASSERT_EQ(fmc_llround(0.45), 0ll);
  ASSERT_EQ(fmc_llround(-0.45), 0ll);
  ASSERT_EQ(fmc_llround(0.55), 1ll);
  ASSERT_EQ(fmc_llround(-0.55), -1ll);
  ASSERT_EQ(fmc_llround(0.95), 1ll);
  ASSERT_EQ(fmc_llround(-0.95), -1ll);

  ASSERT_EQ(fmc_llround(1.1), 1ll);
  ASSERT_EQ(fmc_llround(-1.1), -1ll);
  ASSERT_EQ(fmc_llround(1.45), 1ll);
  ASSERT_EQ(fmc_llround(-1.45), -1ll);
  ASSERT_EQ(fmc_llround(1.55), 2ll);
  ASSERT_EQ(fmc_llround(-1.55), -2ll);
  ASSERT_EQ(fmc_llround(1.95), 2ll);
  ASSERT_EQ(fmc_llround(-1.95), -2ll);
}

TEST(fmc_math, remainder) {
  ASSERT_EQ(fmc_remainder(0.0), 0.0);
  ASSERT_EQ(fmc_remainder(0.25), 0.25);
  ASSERT_EQ(fmc_remainder(0.5), -0.5);
  ASSERT_EQ(fmc_remainder(0.75), -0.25);

  ASSERT_EQ(fmc_remainder(1.0), 0.0);
  ASSERT_EQ(fmc_remainder(1.25), 0.25);
  ASSERT_EQ(fmc_remainder(1.5), -0.5);
  ASSERT_EQ(fmc_remainder(1.75), -0.25);

  ASSERT_EQ(fmc_remainder(-0.25), -0.25);
  ASSERT_EQ(fmc_remainder(-0.5), 0.5);
  ASSERT_EQ(fmc_remainder(-0.75), 0.25);

  ASSERT_EQ(fmc_remainder(-1.0), -0.0);
  ASSERT_EQ(fmc_remainder(-1.25), -0.25);
  ASSERT_EQ(fmc_remainder(-1.5), 0.5);
  ASSERT_EQ(fmc_remainder(-1.75), 0.25);
}

TEST(fmc_math, is_epsilon) {
  ASSERT_FALSE(FMC_ALMOST_ZERO(0.1));
  ASSERT_FALSE(FMC_ALMOST_ZERO(-0.1));

  ASSERT_TRUE(FMC_ALMOST_ZERO(0.0));
  ASSERT_TRUE(FMC_ALMOST_ZERO(0x1.p-60));
  ASSERT_TRUE(FMC_ALMOST_ZERO(-0x1.p-60));

  ASSERT_FALSE(FMC_ALMOST_EQUAL(1.0 + 0x1.p-60, 1.1));
  ASSERT_FALSE(FMC_ALMOST_EQUAL(1.0 + -0x1.p-60, 1.1));

  ASSERT_TRUE(FMC_ALMOST_EQUAL(1.0 + 0x1.p-60, 1.0));
  ASSERT_TRUE(FMC_ALMOST_EQUAL(1.0 + -0x1.p-60, 1.0));

  ASSERT_FALSE(FMC_ALMOST_LESS(1.01, 1.0));
  ASSERT_TRUE(FMC_ALMOST_LESS(1.0 + 0x1.p-60, 1.0));
  ASSERT_TRUE(FMC_ALMOST_LESS(1.0, 1.0));
  ASSERT_TRUE(FMC_ALMOST_LESS(1.0 - 0x1.p-60, 1.0));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
