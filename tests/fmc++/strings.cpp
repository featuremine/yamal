/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <fmc++/gtestwrap.hpp>
#include <fmc++/strings.hpp>

TEST(strings, str_to_double_digits) {
  ASSERT_EQ(fmc::from_string_view<double>("0.7660166666666666667").first,
                   0.7660166666666666667);
  ASSERT_EQ(fmc::from_string_view<double>("0.7660166666666667").second,
                   "0.7660166666666667");
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.7660166666666667").first,
                   0.7660166666666667);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.766016666666667").first,
                   0.766016666666667);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.76601666666666").first,
                   0.76601666666666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.7660166666666").first,
                   0.7660166666666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.766016666666").first,
                   0.766016666666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.76601666666").first,
                   0.76601666666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.7660166666").first,
                   0.7660166666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.766016666").first,
                   0.766016666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.76601666").first,
                   0.76601666);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("0.7660166").first, 0.7660166);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("766016666666667").first,
                   766016666666667);
  ASSERT_EQ(fmc::from_string_view<double>("766016666666667.7660166666666666667")
                .second,
                   "766016666666667.766016666666666");
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("12217.6043").first, 12217.6043);
  ASSERT_DOUBLE_EQ(fmc::from_string_view<double>("73344.998").first, 73344.998);
  auto view = std::string_view("99164.1095");
  ASSERT_EQ(fmc::from_string_view<double>(view).second, view);
}

TEST(strings, str_to_double_stdlib) {
  const char *str = "98904.922";
  double x1 = fmc::from_string_view<double>(str).first;
  double x2 = strtod(str, nullptr);
  printf("from_string_view: %.15f\n", x1);
  printf("strtod:           %.15f\n", x2);
  ASSERT_EQ(x1, x2);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
