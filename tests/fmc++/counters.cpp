/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include "fmc++/gtestwrap.hpp"

#define FMC_COUNTER_ENABLE
#include "fmc++/counters.hpp"

#include <chrono>
#include <cmath>
#include <thread>

TEST(counters, counters) {
  using namespace fmc::counter;
  using namespace std;
  counter_record<rdtsc_avg> counter("test");

  for (auto i = 0; i < 100; ++i) {
    counter.start();
    std::this_thread::sleep_for(1ms);
    counter.stop();
  }
  auto err = fabs(1000000.0 - counter.value()) / 10000.0;
  printf("counter measurement reports error %.2f%%\n", err);
  ASSERT_LT(err, 100.0);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
