/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
