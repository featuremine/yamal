/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc++/gtestwrap.hpp>
#include <fmc++/shared_map.hpp>

const int initial_value = 1000000;
const int test_count = 500000;

struct TestValue {
  int value = initial_value;
};

class V {};

TEST(shared_map, concurrency_1) {
  fmc::shared_map<int, TestValue> view_w;
  fmc::shared_map<int, TestValue> view_r(view_w);

  std::atomic<int> current_key = 0;

  std::thread reader([&current_key, &view_r]() {
    do {
      auto key = current_key.load();
      if (key == test_count)
        break;
      auto it = view_r.find(key);
      if (it != view_r.end()) {
        TestValue &v = (*it).second;
        ASSERT_GE(v.value, initial_value);
        ASSERT_LE(v.value, initial_value + test_count);
      }
    } while (true);
  });

  std::thread writer([&current_key, &view_w]() {
    for (int i = 0; i < test_count; ++i) {
      auto testValue = std::make_unique<TestValue>();
      testValue->value += i;
      ++current_key;
      auto it = view_w.insert(current_key, std::move(testValue));
      ASSERT_TRUE(it.second);
    }
  });
  reader.join();
  writer.join();
  {
    auto it = view_w.insert(1, std::make_unique<TestValue>());
    ASSERT_FALSE(it.second);
  }
}

TEST(shared_map, single_1) {
  fmc::shared_map<int, TestValue> view_w;
  fmc::shared_map<int, TestValue> view_r(view_w);
  const fmc::shared_map<int, TestValue> &view_cr = view_r;

  auto set = [&view_w](int key, int value) {
    auto ptr = std::make_unique<TestValue>();
    ptr->value = value;
    view_w.insert(key, std::move(ptr));
  };

  auto read = [&view_r](int key, const auto &reader) {
    {
      auto it = view_r.find(key);
      reader(it != view_r.end() ? &(*it).second : nullptr);
    }
  };

  auto cread = [&view_cr](int key, const auto &reader) {
    {
      auto it = view_cr.find(key);
      reader(it != view_cr.end() ? &(*it).second : nullptr);

      using T_REF = decltype((*it).second);
      using T = typename std::remove_reference<T_REF>::type;
      static_assert(std::is_const<T>::value);
    }
  };

  read(10, [](TestValue *info) { ASSERT_EQ(info, nullptr); });
  read(9, [](TestValue *info) { ASSERT_EQ(info, nullptr); });

  set(9, 300);

  read(10, [](TestValue *info) { ASSERT_EQ(info, nullptr); });
  read(9, [](TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 300);
  });

  set(10, 301);

  read(10, [](TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 301);
  });
  read(9, [](TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 300);
  });

  set(9, 302);

  read(10, [](TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 301);
  });
  read(9, [](TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 300);
  });

  EXPECT_TRUE(view_w.contains(9));
  EXPECT_TRUE(view_w.contains(10));
  EXPECT_FALSE(view_w.contains(11));

  cread(11, [](const TestValue *info) { ASSERT_EQ(info, nullptr); });
  cread(10, [](const TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 301);
  });
  cread(9, [](const TestValue *info) {
    ASSERT_NE(info, nullptr);
    ASSERT_EQ(info->value, 300);
  });
}
