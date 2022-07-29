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

#include "fmc++/gtestwrap.hpp"
#include "fmc/files.h"

#include "fmc++/config/serialize.hpp"
#include "fmc++/config/variant_map.hpp"

TEST(var_config, standalone_long_node) {
  fmc::configs::variant_map::node long_node_((long)5);
  ASSERT_EQ(long_node_.as<long>(), 5);
  ASSERT_EQ(long_node_.as<unsigned>(), 5);
  ASSERT_EQ(long_node_.as<int>(), 5);
  ASSERT_EQ(std::string("5").compare(long_node_.str()), 0);
  ASSERT_EQ(long_node_.as<std::string>().compare("5"), 0);
  ASSERT_THROW(long_node_.as<bool>(), std::runtime_error);
  ASSERT_DOUBLE_EQ(long_node_.as<double>(), 5.0);
  ASSERT_THROW(long_node_.as<fmc::time>(), std::runtime_error);
  ASSERT_THROW(long_node_.to_d(), std::runtime_error);
  ASSERT_THROW(long_node_.to_a(), std::runtime_error);
}

TEST(var_config, standalone_int_node) {
  fmc::configs::variant_map::node int_node_((int)5);
  ASSERT_EQ(int_node_.as<long>(), 5);
  ASSERT_EQ(int_node_.as<unsigned>(), 5);
  ASSERT_EQ(int_node_.as<int>(), 5);
  ASSERT_EQ(int_node_.as<std::string>().compare("5"), 0);
  ASSERT_THROW(int_node_.as<bool>(), std::runtime_error);
  ASSERT_DOUBLE_EQ(int_node_.as<double>(), 5.0);
  ASSERT_THROW(int_node_.as<fmc::time>(), std::runtime_error);
  ASSERT_THROW(int_node_.to_d(), std::runtime_error);
  ASSERT_THROW(int_node_.to_a(), std::runtime_error);
  ASSERT_EQ(std::string("5").compare(int_node_.str()), 0);
}

TEST(var_config, standalone_unsigned_node) {
  fmc::configs::variant_map::node unsigned_node_((unsigned)5);
  ASSERT_EQ(unsigned_node_.as<long>(), 5);
  ASSERT_EQ(unsigned_node_.as<unsigned>(), 5);
  ASSERT_EQ(unsigned_node_.as<int>(), 5);
  ASSERT_EQ(unsigned_node_.as<std::string>().compare("5"), 0);
  ASSERT_THROW(unsigned_node_.as<bool>(), std::runtime_error);
  ASSERT_DOUBLE_EQ(unsigned_node_.as<double>(), 5.0);
  ASSERT_THROW(unsigned_node_.as<fmc::time>(), std::runtime_error);
  ASSERT_THROW(unsigned_node_.to_d(), std::runtime_error);
  ASSERT_THROW(unsigned_node_.to_a(), std::runtime_error);
  ASSERT_EQ(std::string("5").compare(unsigned_node_.str()), 0);
}

TEST(var_config, standalone_bool_node) {
  fmc::configs::variant_map::node bool_node_(false);
  ASSERT_EQ(bool_node_.as<bool>(), false);
  ASSERT_THROW(bool_node_.as<std::string>(), std::runtime_error);
  ASSERT_THROW(bool_node_.as<int>(), std::runtime_error);
  ASSERT_THROW(bool_node_.as<long>(), std::runtime_error);
  ASSERT_THROW(bool_node_.as<unsigned>(), std::runtime_error);
  ASSERT_THROW(bool_node_.as<double>(), std::runtime_error);
  ASSERT_THROW(bool_node_.as<fmc::time>(), std::runtime_error);
  ASSERT_THROW(bool_node_.to_d(), std::runtime_error);
  ASSERT_THROW(bool_node_.to_a(), std::runtime_error);
  ASSERT_EQ(std::string("False").compare(bool_node_.str()), 0);
}

TEST(var_config, standalone_double_node) {
  fmc::configs::variant_map::node double_node_((double)5.36);
  ASSERT_DOUBLE_EQ(double_node_.as<double>(), 5.36);
  ASSERT_EQ(double_node_.as<std::string>().compare("5.360000"), 0);
  ASSERT_THROW(double_node_.as<int>(), std::runtime_error);
  ASSERT_THROW(double_node_.as<long>(), std::runtime_error);
  ASSERT_THROW(double_node_.as<bool>(), std::runtime_error);
  ASSERT_THROW(double_node_.as<unsigned>(), std::runtime_error);
  ASSERT_THROW(double_node_.as<fmc::time>(), std::runtime_error);
  ASSERT_THROW(double_node_.to_d(), std::runtime_error);
  ASSERT_THROW(double_node_.to_a(), std::runtime_error);
  ASSERT_EQ(std::string("5.360000").compare(double_node_.str()), 0);
}

TEST(var_config, standalone_time_node) {
  fmc::configs::variant_map::node time_node_(fmc::time(0));
  ASSERT_EQ(time_node_.as<fmc::time>(), fmc::time(0));
  ASSERT_THROW(time_node_.as<std::string>(), std::runtime_error);
  ASSERT_THROW(time_node_.as<int>(), std::runtime_error);
  ASSERT_THROW(time_node_.as<long>(), std::runtime_error);
  ASSERT_THROW(time_node_.as<bool>(), std::runtime_error);
  ASSERT_THROW(time_node_.as<double>(), std::runtime_error);
  ASSERT_THROW(time_node_.as<unsigned>(), std::runtime_error);
  ASSERT_THROW(time_node_.to_d(), std::runtime_error);
  ASSERT_THROW(time_node_.to_a(), std::runtime_error);
  ASSERT_EQ(
      std::string("1970-01-01 00:00:00.000000000").compare(time_node_.str()),
      0);
}

TEST(var_config, standalone_section) {
  fmc::configs::variant_map::section section_;
  auto &my_key_node = section_[std::string_view("my_key")];
  my_key_node = fmc::configs::variant_map::node((double)5.36);
  ASSERT_EQ(my_key_node.type(), fmc::configs::interface::node::FLOATING_POINT);
  ASSERT_DOUBLE_EQ(my_key_node.as<double>(), 5.36);

  fmc::configs::variant_map::section upper_section_;
  auto &my_upper_section_key_node = upper_section_[std::string_view("my_key")];
  my_upper_section_key_node = std::move(section_);

  ASSERT_DOUBLE_EQ(
      upper_section_[std::string_view("my_key")][std::string_view("my_key")]
          .as<double>(),
      5.36);
}

TEST(var_config, standalone_array) {
  fmc::configs::variant_map::array array_;
  ASSERT_THROW(array_[0], std::runtime_error);
  array_.push_back(fmc::configs::variant_map::node((double)5.36));
  auto &my_key_node = array_[0];
  ASSERT_EQ(my_key_node.type(), fmc::configs::interface::node::FLOATING_POINT);
  ASSERT_DOUBLE_EQ(my_key_node.as<double>(), 5.36);
}

TEST(var_config, populate_from_ini) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  std::string_view test_data = ""
                               "[SECTION1]"
                               "\n"
                               "test=first"
                               "\n"
                               "num=1"
                               "\n"
                               "\n"
                               "[SECTION2]"
                               "\r\n"
                               "test=second"
                               "\r\n"
                               "\r\n"
                               "num=1.5"
                               "\r\n"
                               "[SECTION3]"
                               "\n"
                               "test=third"
                               "\n"
                               "num=-9";
  write(fd, test_data.data(), test_data.size());
  lseek(fd, 0, SEEK_SET);

  fmc::configs::variant_map::node data =
      fmc::configs::serialize::variant_map_load_ini(fd, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(data.type(), fmc::configs::interface::node::SECTION);

  auto &data_as_section = data.to_d();

  ASSERT_TRUE(data_as_section.has("SECTION1"));

  auto &section_one_node = data_as_section["SECTION1"];

  ASSERT_EQ(section_one_node.type(), fmc::configs::interface::node::SECTION);

  auto &section_one = section_one_node.to_d();

  ASSERT_TRUE(section_one.has("test"));
  ASSERT_EQ(section_one["test"].type(), fmc::configs::interface::node::STRING);
  ASSERT_EQ(section_one["test"].to_s().compare("first"), 0);
  ASSERT_THROW(section_one["test"].as<int>(), std::runtime_error);
  ASSERT_THROW(section_one["test"].as<long>(), std::runtime_error);
  ASSERT_THROW(section_one["test"].as<unsigned>(), std::runtime_error);
  ASSERT_THROW(section_one["test"].as<double>(), std::runtime_error);
  ASSERT_THROW(section_one["test"].as<fmc::time>(), std::runtime_error);

  ASSERT_TRUE(section_one.has("num"));
  ASSERT_EQ(section_one["num"].type(), fmc::configs::interface::node::STRING);
  ASSERT_EQ(section_one["num"].to_s().compare("1"), 0);
  ASSERT_EQ(section_one["num"].as<int>(), 1);
  ASSERT_EQ(section_one["num"].as<long>(), 1);
  ASSERT_EQ(section_one["num"].as<unsigned>(), 1);
  ASSERT_DOUBLE_EQ(section_one["num"].as<double>(), 1.0);
  ASSERT_THROW(section_one["num"].as<fmc::time>(), std::runtime_error);

  ASSERT_TRUE(data_as_section.has("SECTION2"));

  auto &section_two_node = data_as_section["SECTION2"];

  ASSERT_EQ(section_two_node.type(), fmc::configs::interface::node::SECTION);

  auto &section_two = section_two_node.to_d();

  ASSERT_TRUE(section_two.has("test"));
  ASSERT_EQ(section_two["test"].type(), fmc::configs::interface::node::STRING);
  ASSERT_EQ(section_two["test"].to_s().compare("second"), 0);
  ASSERT_THROW(section_two["test"].as<int>(), std::runtime_error);
  ASSERT_THROW(section_two["test"].as<long>(), std::runtime_error);
  ASSERT_THROW(section_two["test"].as<unsigned>(), std::runtime_error);
  ASSERT_THROW(section_two["test"].as<double>(), std::runtime_error);
  ASSERT_THROW(section_two["test"].as<fmc::time>(), std::runtime_error);

  ASSERT_TRUE(section_two.has("num"));
  ASSERT_EQ(section_two["num"].type(), fmc::configs::interface::node::STRING);
  ASSERT_EQ(section_two["num"].to_s().compare("1.5"), 0);
  ASSERT_DOUBLE_EQ(section_two["num"].as<double>(), 1.5);
  ASSERT_THROW(section_two["num"].as<int>(), std::runtime_error);
  ASSERT_THROW(section_two["num"].as<long>(), std::runtime_error);
  ASSERT_THROW(section_two["num"].as<unsigned>(), std::runtime_error);
  ASSERT_THROW(section_two["num"].as<fmc::time>(), std::runtime_error);

  ASSERT_TRUE(data_as_section.has("SECTION3"));

  auto &section_three_node = data_as_section["SECTION3"];

  ASSERT_EQ(section_three_node.type(), fmc::configs::interface::node::SECTION);

  auto &section_three = section_three_node.to_d();

  ASSERT_TRUE(section_three.has("test"));
  ASSERT_EQ(section_three["test"].type(),
            fmc::configs::interface::node::STRING);
  ASSERT_EQ(section_three["test"].to_s().compare("third"), 0);
  ASSERT_THROW(section_three["test"].as<int>(), std::runtime_error);
  ASSERT_THROW(section_three["test"].as<long>(), std::runtime_error);
  ASSERT_THROW(section_three["test"].as<unsigned>(), std::runtime_error);
  ASSERT_THROW(section_three["test"].as<double>(), std::runtime_error);
  ASSERT_THROW(section_three["test"].as<fmc::time>(), std::runtime_error);

  ASSERT_TRUE(section_three.has("num"));
  ASSERT_EQ(section_three["num"].type(), fmc::configs::interface::node::STRING);
  ASSERT_EQ(section_three["num"].to_s().compare("-9"), 0);
  ASSERT_EQ(section_three["num"].as<int>(), -9);
  ASSERT_EQ(section_three["num"].as<long>(), -9);
  ASSERT_DOUBLE_EQ(section_three["num"].as<double>(), -9.0);
  ASSERT_THROW(section_three["num"].as<unsigned>(), std::runtime_error);
  ASSERT_THROW(section_three["num"].as<fmc::time>(), std::runtime_error);
}

TEST(var_config, bad_ini) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  std::string_view test_data = "test=first"
                               "\n"
                               "num=1"
                               "\n"
                               "num=-9";
  write(fd, test_data.data(), test_data.size());
  lseek(fd, 0, SEEK_SET);

  fmc::configs::variant_map::node data =
      fmc::configs::serialize::variant_map_load_ini(fd, &error);
  ASSERT_NE(error, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
