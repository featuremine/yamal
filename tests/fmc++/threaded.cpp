/******************************************************************************
    COPYRIGHT (c) 2020 by Featuremine Corporation.
    This software has been provided pursuant to a License Agreement
    containing restrictions on its use.  This software contains
    valuable trade secrets and proprietary information of
    FeatureMine Corporation and is protected by law.  It may not be
    copied or distributed in any form or medium, disclosed to third
    parties, reverse engineered or used in any manner not provided
    for in said License Agreement except with the prior written
    authorization Featuremine Corporation.
*****************************************************************************/

/**
 * @file threaded.cpp
 * @author Federico Ravchina
 * @date 22 Jul 2021
 * @brief File contains tests for FMC threaded
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc++/threaded.hpp>

const int initial_value = 1000000;
const int test_count = 500000;

struct TestValue {
  int value = initial_value;
};

TEST(threaded, concurrent_1) {
  fmc::threaded<TestValue> value;

  std::atomic<bool> done = false;

  std::thread reader([&done, &value]() {
    while (!done.load()) {
      auto &v = value.consume();
      ASSERT_GE(v.value, initial_value);
      ASSERT_LE(v.value, initial_value + test_count);
      value.cleanup();
    }
  });

  std::thread writer([&done, &value]() {
    for (int i = 0; i < test_count; ++i) {
      auto &v = value.reserve();
      ++v.value;
      value.commit();
    }
    done = true;
  });
  reader.join();
  writer.join();

  auto &v = value.consume();
  ASSERT_EQ(v.value, initial_value + test_count);
}

TEST(threaded, sequential_1) {
  fmc::threaded<TestValue> value;

  for (int i = 0; i < test_count; ++i) {
    {
      auto &v = value.reserve();
      ++v.value;
      value.commit();
    }
    {
      auto &v = value.consume();
      ASSERT_GE(v.value, initial_value);
      ASSERT_LE(v.value, initial_value + test_count);
      value.cleanup();
    }
  }

  auto &v = value.consume();
  ASSERT_EQ(v.value, initial_value + test_count);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
