/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
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
 * @file threaded.cpp
 * @date 22 Jul 2021
 * @brief File contains tests for FMC threaded
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <fmc++/threaded.hpp>

const int threaded_initial_value = 1000000;
const int threaded_test_count = 500000;

struct TheadedTestValue {
  int value = threaded_initial_value;
};

TEST(threaded, concurrent_1) {
  fmc::threaded<TheadedTestValue> value;

  std::atomic<bool> done = false;

  std::thread reader([&done, &value]() {
    while (!done.load()) {
      auto &v = value.consume();
      ASSERT_GE(v.value, threaded_initial_value);
      ASSERT_LE(v.value, threaded_initial_value + threaded_test_count);
      value.cleanup();
    }
  });

  std::thread writer([&done, &value]() {
    for (int i = 0; i < threaded_test_count; ++i) {
      auto &v = value.reserve();
      ++v.value;
      value.commit();
    }
    done = true;
  });
  reader.join();
  writer.join();

  auto &v = value.consume();
  ASSERT_EQ(v.value, threaded_initial_value + threaded_test_count);
}

TEST(threaded, sequential_1) {
  fmc::threaded<TheadedTestValue> value;

  for (int i = 0; i < threaded_test_count; ++i) {
    {
      auto &v = value.reserve();
      ++v.value;
      value.commit();
    }
    {
      auto &v = value.consume();
      ASSERT_GE(v.value, threaded_initial_value);
      ASSERT_LE(v.value, threaded_initial_value + threaded_test_count);
      value.cleanup();
    }
  }

  auto &v = value.consume();
  ASSERT_EQ(v.value, threaded_initial_value + threaded_test_count);
}
