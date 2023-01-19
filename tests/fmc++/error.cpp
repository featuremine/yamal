/******************************************************************************

        COPYRIGHT (c) 2023 by Featuremine Corporation.
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
 * @file error.cpp
 * @author Federico Ravchina
 * @date 18 Jan 2023
 * @brief File contains tests for FMC++ error API
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/error.hpp>

#include <fmc++/gtestwrap.hpp>

#include <sstream>

TEST(error, cat) {
  fmc::error error;
  auto get_stream_output = [&]() {
    std::stringbuf buffer;
    std::ostream os(&buffer);
    os << error;
    return buffer.str();
  };
  auto get_tostring = [&]() { return std::to_string(error); };
  auto get_msg = [&]() { return std::string_view(error.c_str()); };

  EXPECT_FALSE(error);
  EXPECT_EQ(get_msg(), "None");
  EXPECT_EQ(get_stream_output(), "None");
  EXPECT_EQ(get_tostring(), "None");

  error.sprintf("%d %d", 1, 2);
  EXPECT_TRUE(error);
  EXPECT_EQ(get_msg(), "1 2");
  EXPECT_EQ(get_stream_output(), "1 2");
  EXPECT_EQ(get_tostring(), "1 2");

  error.sprintf("%d %d", 3, 4);
  EXPECT_TRUE(error);
  EXPECT_EQ(get_msg(), "3 4");
  EXPECT_EQ(get_stream_output(), "3 4");
  EXPECT_EQ(get_tostring(), "3 4");

  error = "5 6";
  EXPECT_TRUE(error);
  EXPECT_EQ(get_msg(), "5 6");
  EXPECT_EQ(get_stream_output(), "5 6");
  EXPECT_EQ(get_tostring(), "5 6");

  error = nullptr;
  EXPECT_FALSE(error);
  EXPECT_EQ(get_msg(), "None");
  EXPECT_EQ(get_stream_output(), "None");
  EXPECT_EQ(get_tostring(), "None");

  error = "7 8";
  EXPECT_TRUE(error);
  EXPECT_EQ(get_msg(), "7 8");
  EXPECT_EQ(get_stream_output(), "7 8");
  EXPECT_EQ(get_tostring(), "7 8");
}

TEST(error, move_copy) {
  std::optional<fmc::error> error;
  auto get_msg = [&]() -> std::string_view {
    if (!error) {
      return "<empty>";
    }
    return std::string_view(error.value().c_str());
  };

  EXPECT_EQ(get_msg(), "<empty>");

  fmc::error error2 = "error2";
  error.emplace(error2);
  EXPECT_EQ(get_msg(), "error2");
  error.reset();

  fmc::error error3 = "error3";
  error.emplace(std::move(error3));
  EXPECT_EQ(get_msg(), "error3");
  error.reset();

  error.emplace("error");
  fmc::error error4(std::move(error.value()));
  EXPECT_EQ(get_msg(), "None");
  error.reset();

  error.emplace("error");
  fmc::error error5(error.value());
  EXPECT_EQ(get_msg(), "error");
  error.reset();

  error.emplace();
  fmc::error error6 = "error6";
  error.value() = error6;
  EXPECT_EQ(get_msg(), "error6");
  error.reset();

  error.emplace();
  fmc::error error7 = "error7";
  error.value() = std::move(error7);
  EXPECT_EQ(get_msg(), "error7");
  error.reset();

  error.emplace("error");
  fmc::error error8;
  error8 = std::move(error.value());
  EXPECT_EQ(get_msg(), "None");
  error.reset();

  error.emplace("error");
  fmc::error error9;
  error9 = error.value();
  EXPECT_EQ(get_msg(), "error");
  error.reset();
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
