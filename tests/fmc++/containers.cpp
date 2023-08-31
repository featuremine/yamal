/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc++/lazy_rem_vector.hpp>
#include <fmc++/stable_map.hpp>

#include <fmc++/gtestwrap.hpp>

TEST(stable_map, test_1) {
  std::vector<int *> values(8192);

  fmc::stable_map<int, int> map;

  ASSERT_EQ(map.begin(), map.end());

  for (size_t i = 0u; i < values.size(); ++i) {
    values[i] = &map[i];
  }
  for (size_t i = 0u; i < values.size(); ++i) {
    ASSERT_EQ(values[i], &map.find(i)->second);
    ASSERT_EQ(values[i], &map[i]);
  }
  ASSERT_NE(map.begin(), map.end());
  ASSERT_NE(map.find(0), map.end());
  ASSERT_EQ(&map.find(0)->second, values[0]);
  ASSERT_EQ(&map.find(5)->second, values[5]);
  ASSERT_EQ(map.find(99999), map.end());
}

TEST(lazy_rem_vector, test_1) {
  fmc::lazy_rem_vector<int> vec;

  fmc::push_unique(vec, 1);
  fmc::push_unique(vec, 2);
  fmc::push_unique(vec, 4);
  fmc::push_unique(vec, 5);
  fmc::push_unique(vec, 1);
  fmc::push_unique(vec, 2);

  auto it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 2);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 4);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_EQ(it, vec.end());

  vec.lock();
  std::erase_if(vec, [&](const int &val) { return val == 4 || val == 2; });

  it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_TRUE(it.was_removed());
  ASSERT_EQ(*it, 2);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_TRUE(it.was_removed());
  ASSERT_EQ(*it, 4);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_EQ(it, vec.end());
  vec.release();

  it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_EQ(it, vec.end());

  fmc::push_unique(vec, 2);
  fmc::push_unique(vec, 4);

  it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 2);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 4);
  ++it;
  ASSERT_EQ(it, vec.end());

  vec.lock();
  std::erase_if(vec, [&](const int &val) { return val == 5; });

  it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_TRUE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 2);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 4);
  ++it;
  ASSERT_EQ(it, vec.end());

  fmc::push_unique(vec, 5);

  it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 2);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 4);
  ++it;
  ASSERT_EQ(it, vec.end());

  vec.release();

  it = vec.begin();
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 1);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 5);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 2);
  ++it;
  ASSERT_NE(it, vec.end());
  ASSERT_FALSE(it.was_removed());
  ASSERT_EQ(*it, 4);
  ++it;
  ASSERT_EQ(it, vec.end());
}

#include "shared_map.cpp"
#include "static_vector.cpp"
#include "threaded.cpp"
#include "variant_map.cpp"

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
