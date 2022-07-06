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

#include "../src/timeline.hpp"

#include "gtestwrap.hpp"

TEST(stable_map, test_1) {
  std::vector<int *> values(8192);

  stable_map<int, int> map;

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
  lazy_rem_vector<int> vec;

  vec.push_unique(1);
  vec.push_unique(2);
  vec.push_unique(4);
  vec.push_unique(5);
  vec.push_unique(1);
  vec.push_unique(2);

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
  vec.erase_if([&](const int &val) { return val == 4 || val == 2; });

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

  vec.push_unique(2);
  vec.push_unique(4);

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
  vec.erase_if([&](const int &val) { return val == 5; });

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

  vec.push_unique(5);

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

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
