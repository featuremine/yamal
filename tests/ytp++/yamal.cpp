/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file yamal.cpp
 * @author Andres Rangel
 * @date 22 Sep 2022
 * @brief File contains tests for YTP yamal layer
 *
 * @see http://www.featuremine.com
 */

#include <fmc++/gtestwrap.hpp>
#include <ytp++/yamal.hpp>

using namespace ytp;

TEST(yamal, data_closable) {

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal yamal = ytp::yamal(fd, true);
  ytp::data data = yamal.data();

  ASSERT_TRUE(data.closable());
  ASSERT_FALSE(data.closed());
  data.close();
  ASSERT_TRUE(data.closed());

}

TEST(yamal, data_unclosable) {

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal yamal = ytp::yamal(fd, false);
  ytp::data data = yamal.data();

  ASSERT_FALSE(data.closable());
  ASSERT_FALSE(data.closed());
  ASSERT_THROW(data.close(), std::runtime_error);

}

TEST(yamal, yamal_streams) {

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal yamal = ytp::yamal(fd);
  ytp::streams streams = yamal.streams();

  stream s = streams.announce("peer1", "ch1", "encoding1");
  ASSERT_NE(s.id(), 0);
  ASSERT_THROW(streams.announce("peer1", "ch1", "invalid"), std::runtime_error);
  auto [ls, lsenc] = *streams.lookup("peer1", "ch1");
  ASSERT_EQ(s, ls);
  ASSERT_FALSE(streams.lookup("peer1", "invalid"));
  ASSERT_FALSE(streams.lookup("invalid", "ch1"));
  auto [sseqn, speer, sch, sencoding] = yamal.announcement(s);
  ASSERT_EQ(sseqn, 1);
  ASSERT_EQ(speer, "peer1");
  ASSERT_EQ(sch, "ch1");
  ASSERT_EQ(sencoding, "encoding1");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

