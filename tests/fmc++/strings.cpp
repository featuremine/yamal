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

#include <fmc++/gtestwrap.hpp>
#include <fmc++/strings.hpp>

TEST(strings, str_to_double_digits) {
  ASSERT_EQ(fmc::_from_string_view_double("0.7660166666666667").second, "0.7660166666666667");
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.7660166666666667").first, 0.7660166666666667);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.766016666666667").first, 0.766016666666667);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.76601666666666").first, 0.76601666666666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.7660166666666").first, 0.7660166666666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.766016666666").first, 0.766016666666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.76601666666").first, 0.76601666666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.7660166666").first, 0.7660166666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.766016666").first, 0.766016666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.76601666").first, 0.76601666);
  ASSERT_DOUBLE_EQ(fmc::_from_string_view_double("0.7660166").first, 0.7660166);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
