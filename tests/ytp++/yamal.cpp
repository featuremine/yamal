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
#include <ostream>
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

  ytp::yamal_t yamal = ytp::yamal_t(fd, true);
  ytp::data_t data = yamal.data();

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

  ytp::yamal_t yamal = ytp::yamal_t(fd, false);
  ytp::data_t data = yamal.data();

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

  ytp::yamal_t yamal = ytp::yamal_t(fd);
  ytp::streams_t streams = yamal.streams();

  ytp::stream_t s = streams.announce("peer1", "ch1", "encoding1");
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

TEST(yamal, iteration) {

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal_t yamal = ytp::yamal_t(fd, true);
  ytp::data_t data = yamal.data();

  ASSERT_EQ(ytp::data_t::iterator(), ytp::data_t::iterator());
  ASSERT_EQ(ytp::data_t::reverse_iterator(), ytp::data_t::reverse_iterator());

  ASSERT_TRUE(data.closable());

  ytp::streams_t streams = yamal.streams();
  ytp::stream_t stream = streams.announce("peer", "channel", "encoding");
  ytp::stream_t stream_other = streams.announce("peer2", "channel", "encoding");
  ytp::stream_t stream_other2 =
      streams.announce("peer", "channel2", "encoding");

  ASSERT_EQ(data.begin(), data.end());
  ASSERT_EQ(data.rbegin(), data.rend());

  auto ptr = data.reserve(4);
  memcpy(ptr.data(), "msg1", 4);
  data.commit(1, stream, ptr);

  ptr = data.reserve(4);
  memcpy(ptr.data(), "msg2", 4);
  data.commit(2, stream, ptr);

  ptr = data.reserve(4);
  memcpy(ptr.data(), "msg3", 4);
  data.commit(3, stream, ptr);

  // Forward:

  auto it = data.begin();
  ASSERT_EQ(data.seek((ytp_mmnode_offs)it), it);
  ASSERT_NE(it, data.end());
  auto [seqno1, ts1, stream1, data1] = *it;
  ASSERT_EQ(seqno1, 1);
  ASSERT_EQ(ts1, 1);
  ASSERT_EQ(stream1, stream);
  ASSERT_NE(stream1, stream_other);
  ASSERT_NE(stream1, stream_other2);
  ASSERT_EQ(data1, "msg1");
  ++it;
  ASSERT_EQ(data.seek((ytp_mmnode_offs)it), it);
  auto [seqno2, ts2, stream2, data2] = *it;
  ASSERT_EQ(seqno2, 2);
  ASSERT_EQ(ts2, 2);
  ASSERT_EQ(stream2, stream);
  ASSERT_NE(stream2, stream_other);
  ASSERT_NE(stream2, stream_other2);
  ASSERT_EQ(data2, "msg2");
  ++it;
  ASSERT_EQ(data.seek((ytp_mmnode_offs)it), it);
  auto [seqno3, ts3, stream3, data3] = *it;
  ASSERT_EQ(seqno3, 3);
  ASSERT_EQ(ts3, 3);
  ASSERT_EQ(stream3, stream);
  ASSERT_NE(stream3, stream_other);
  ASSERT_NE(stream3, stream_other2);
  ASSERT_EQ(data3, "msg3");
  ASSERT_NE(it, data.end());
  ++it;
  ASSERT_EQ(data.seek((ytp_mmnode_offs)it), it);
  ASSERT_EQ(it, data.end());

  // Reverse:
  auto rit = data.rbegin();
  ASSERT_EQ(data.rseek((ytp_mmnode_offs)rit), rit);
  ASSERT_NE(rit, data.rend());
  auto [rseqno1, rts1, rstream1, rdata1] = *rit;
  ASSERT_EQ(rseqno1, 3);
  ASSERT_EQ(rts1, 3);
  ASSERT_EQ(rstream1, stream);
  ASSERT_NE(rstream1, stream_other);
  ASSERT_NE(rstream1, stream_other2);
  ASSERT_EQ(rdata1, "msg3");
  ++rit;
  ASSERT_EQ(data.rseek((ytp_mmnode_offs)rit), rit);
  auto [rseqno2, rts2, rstream2, rdata2] = *rit;
  ASSERT_EQ(rseqno2, 2);
  ASSERT_EQ(rts2, 2);
  ASSERT_EQ(rstream2, stream);
  ASSERT_NE(rstream2, stream_other);
  ASSERT_NE(rstream2, stream_other2);
  ASSERT_EQ(rdata2, "msg2");
  ++rit;
  ASSERT_EQ(data.rseek((ytp_mmnode_offs)rit), rit);
  auto [rseqno3, rts3, rstream3, rdata3] = *rit;
  ASSERT_EQ(rseqno3, 1);
  ASSERT_EQ(rts3, 1);
  ASSERT_EQ(rstream3, stream);
  ASSERT_NE(rstream3, stream_other);
  ASSERT_NE(rstream3, stream_other2);
  ASSERT_EQ(rdata3, "msg1");
  ASSERT_NE(rit, data.rend());
  ++rit;
  ASSERT_EQ(data.rseek((ytp_mmnode_offs)rit), rit);
  ASSERT_EQ(rit, data.rend());
}

TEST(yamal, serialization) {

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal_t yamal = ytp::yamal_t(fd, true);
  ytp::streams_t streams = yamal.streams();
  ytp::stream_t stream = streams.announce("peer", "channel", "encoding");

  std::ostringstream os;
  os << stream;

  std::string sstream = os.str();
  ASSERT_EQ(sstream, std::to_string(stream.id()));
  ASSERT_EQ(sstream, "48");
}

TEST(yamal, hashing) {

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal_t yamal = ytp::yamal_t(fd, true);
  ytp::streams_t streams = yamal.streams();
  ytp::stream_t stream = streams.announce("peer", "channel", "encoding");

  size_t shash = std::hash<ytp::stream_t>{}(stream);
  size_t rawhash = std::hash<ytp_mmnode_offs>{}(stream.id());
  ASSERT_EQ(shash, rawhash);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
