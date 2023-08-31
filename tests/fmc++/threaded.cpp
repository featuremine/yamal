/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
