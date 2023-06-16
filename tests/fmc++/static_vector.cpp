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
 * @file static_vector.cpp
 * @date Oct 25 2022
 * @brief File contains tests for static_vector
 *
 * @see http://www.featuremine.com
 */

#include "fmc++/static_vector.hpp"
#include <fmc++/gtestwrap.hpp>

#include <unordered_map>

struct test_counters {
  std::size_t constructed = 0;
  std::size_t destroyed = 0;
  std::size_t copied_from = 0;
  std::size_t copied_to = 0;
  std::size_t moved_from = 0;
  std::size_t moved_to = 0;
  std::size_t assigned_copy_from = 0;
  std::size_t assigned_copy_to = 0;
  std::size_t assigned_move_from = 0;
  std::size_t assigned_move_to = 0;
};

static std::unordered_map<void *, test_counters> counters;

struct test_type {
  test_type(std::size_t value) : value(value) {
    auto &c = counters[this];
    ++c.constructed;
  }
  test_type(const test_type &from) : value(from.value) {
    auto &c = counters[this];
    auto &cfrom = counters[const_cast<test_type *>(&from)];
    ++c.copied_to;
    ++cfrom.copied_from;
  }
  test_type(test_type &&from) : value(from.value) {
    auto &c = counters[this];
    auto &cfrom = counters[const_cast<test_type *>(&from)];
    from.value = 0;
    ++c.moved_to;
    ++cfrom.moved_from;
  }
  test_type &operator=(const test_type &from) {
    auto &c = counters[this];
    auto &cfrom = counters[const_cast<test_type *>(&from)];
    value = from.value;
    ++c.assigned_copy_to;
    ++cfrom.assigned_copy_from;
    return *this;
  }
  test_type &operator=(test_type &&from) {
    auto &c = counters[this];
    auto &cfrom = counters[const_cast<test_type *>(&from)];
    value = from.value;
    from.value = 0;
    ++c.assigned_move_to;
    ++cfrom.assigned_move_from;
    return *this;
  }
  ~test_type() {
    auto &c = counters[this];
    ++c.destroyed;
  }
  std::size_t value;
};

constexpr const std::size_t N = 2;
using static_vector = fmc::static_vector<test_type, N>;

union storage {
  storage() : empty() {}
  ~storage() {}
  char empty;
  static_vector vector;
};

TEST(static_vector, test_type_1) {
  counters.clear();
  storage s;

  new (&s.vector) static_vector();

  EXPECT_EQ(s.vector.size(), 0);
  EXPECT_TRUE(s.vector.empty());

  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);

  s.vector.push_back(test_type(2));
  s.vector.emplace_back(3);

  EXPECT_EQ(s.vector.size(), 2);
  EXPECT_FALSE(s.vector.empty());

  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 2);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 3);

  s.vector.clear();

  EXPECT_EQ(s.vector.size(), 0);
  EXPECT_TRUE(s.vector.empty());

  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 2);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 3);

  s.vector.emplace_back(6);

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 6);

  static_vector bigger({test_type(15), test_type(16)});

  s.vector = bigger;

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 15);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 1);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 16);

  static_vector smaller;
  smaller = {test_type(31)};

  s.vector = smaller;

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 2);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 31);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 2);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 1);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 16);

  s.vector = std::move(bigger);

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 2);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 1);
  EXPECT_EQ(s.vector[0].value, 15);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 2);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 1);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 16);

  s.vector = std::move(smaller);

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 2);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 2);
  EXPECT_EQ(s.vector[0].value, 31);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 3);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 1);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 16);

  EXPECT_TRUE(smaller.empty());
  EXPECT_TRUE(bigger.empty());

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}

TEST(static_vector, move_constructor_1) {
  counters.clear();
  storage s;

  static_vector arg;
  arg.emplace_back(61);
  new (&s.vector) static_vector(std::move(arg));

  EXPECT_EQ(s.vector.size(), 1);
  EXPECT_EQ(arg.size(), 0);
  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 61);

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&arg[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}

TEST(static_vector, copy_constructor_1) {
  counters.clear();
  storage s;

  static_vector arg;
  arg.emplace_back(61);
  new (&s.vector) static_vector(arg);

  EXPECT_EQ(s.vector.size(), 1);
  EXPECT_EQ(arg.size(), 1);
  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 61);

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}

TEST(static_vector, move_assign_1) {
  counters.clear();
  storage s;

  static_vector arg;
  arg.emplace_back(61);
  new (&s.vector) static_vector();
  s.vector = std::move(arg);

  EXPECT_EQ(s.vector.size(), 1);
  EXPECT_EQ(arg.size(), 0);
  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 61);

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&arg[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}

TEST(static_vector, move_assign_2) {
  counters.clear();
  storage s;

  static_vector arg;
  arg.emplace_back(61);
  new (&s.vector) static_vector();
  s.vector.emplace_back(10);
  s.vector.emplace_back(11);
  s.vector = std::move(arg);

  EXPECT_EQ(s.vector.size(), 1);
  EXPECT_EQ(arg.size(), 0);

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 1);
  EXPECT_EQ(s.vector[0].value, 61);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 11);

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&arg[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}

TEST(static_vector, copy_assign_1) {
  counters.clear();
  storage s;

  static_vector copy_constructor_arg;
  copy_constructor_arg.emplace_back(61);
  new (&s.vector) static_vector();
  s.vector = copy_constructor_arg;

  EXPECT_EQ(s.vector.size(), 1);
  EXPECT_EQ(copy_constructor_arg.size(), 1);
  EXPECT_EQ(counters[&s.vector[0]].constructed, 0);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 61);

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}

TEST(static_vector, copy_assign_2) {
  counters.clear();
  storage s;

  static_vector copy_constructor_arg;
  copy_constructor_arg.emplace_back(61);
  new (&s.vector) static_vector();
  s.vector.emplace_back(10);
  s.vector.emplace_back(11);
  s.vector = copy_constructor_arg;

  EXPECT_EQ(s.vector.size(), 1);
  EXPECT_EQ(copy_constructor_arg.size(), 1);

  EXPECT_EQ(counters[&s.vector[0]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[0]].destroyed, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_copy_to, 1);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[0]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[0].value, 61);

  EXPECT_EQ(counters[&s.vector[1]].constructed, 1);
  EXPECT_EQ(counters[&s.vector[1]].destroyed, 1);
  EXPECT_EQ(counters[&s.vector[1]].copied_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].copied_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].moved_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_copy_to, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_from, 0);
  EXPECT_EQ(counters[&s.vector[1]].assigned_move_to, 0);
  EXPECT_EQ(s.vector[1].value, 11);

  s.vector.~static_vector();
  for (std::size_t i = 0; i < N; ++i) {
    auto &c = counters[&s.vector[i]];
    EXPECT_EQ(c.constructed + c.copied_to + c.moved_to, c.destroyed);
  }
}
