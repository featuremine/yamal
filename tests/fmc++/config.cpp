/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

#include "fmc++/gtestwrap.hpp"
#include "fmc/files.h"

#include "fmc++/config/serialize.hpp"
#include "fmc++/config/variant_map.hpp"

TEST(var_config, standalone_section) {
  fmc::configs::variant_map::section section_;
  auto &my_key_node = section_[std::string_view("my_key")];
  my_key_node = fmc::configs::variant_map::node((double)5.36);
  ASSERT_EQ(my_key_node.type(), fmc::configs::interface::node::FLOATING_POINT);
  ASSERT_DOUBLE_EQ(my_key_node.as<double>(), 5.36);

  ASSERT_TRUE(section_.has("my_key"));
  ASSERT_FALSE(section_.has("not_my_key"));

  fmc::configs::variant_map::node node_(std::move(section_));

  ASSERT_TRUE(node_.has("my_key"));
  ASSERT_FALSE(node_.has("not_my_key"));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
