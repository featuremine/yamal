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
 * @file math.cpp
 * @author Federico Ravchina
 * @date 28 Jun 2021
 * @brief File contains tests for FMC math API
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc/math.h>

using namespace std;

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
