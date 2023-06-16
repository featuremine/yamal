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
 * @file stream.cpp
 * @date 8 May 2023
 * @brief File contains tests for YTP stream API
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/cursor.h>
#include <ytp/stream.h>
#include <ytp/streams.h>
#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

#include <list>

#include "stream.hpp"
#include "yamal.hpp"

#include "buildmsg.hpp"
#include "tostr.hpp"

using namespace std;

struct callback_helper {
  struct closure_wrapper_base {
    virtual ~closure_wrapper_base() = default;
  };

  template <typename T> struct closure_wrapper : closure_wrapper_base, T {
    closure_wrapper(T rhs) : T(std::move(rhs)) {}
  };

  template <typename F, typename... Args>
  static void static_cb(void *closure, Args... args) {
    auto &base = *reinterpret_cast<closure_wrapper_base *>(closure);
    auto &wrapped = static_cast<closure_wrapper<F> &>(base);
    F &f = wrapped;
    f(args...);
  }

  template <typename F> std::pair<ytp_cursor_data_cb_t, void *> datacb(F f) {
    auto *cl =
        callbacks
            .emplace_back(std::make_unique<closure_wrapper<F>>(std::move(f)))
            .get();
    return {&static_cb<F, void * closure, uint64_t seqno, int64_t ts,
                       ytp_mmnode_offs stream, size_t sz, const char * data>,
            cl};
  }

  template <typename F> std::pair<ytp_cursor_ann_cb_t, void *> anncb(F f) {
    auto *cl =
        callbacks
            .emplace_back(std::make_unique<closure_wrapper<F>>(std::move(f)))
            .get();
    return {&static_cb<F, ytp_stream_t, uint64_t, size_t, const char *, size_t,
                       const char *, size_t, const char *>,
            cl};
  }

  std::list<std::unique_ptr<closure_wrapper_base>> callbacks;
};

TEST(stream, main_test_1) {
  callback_helper helper;

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *cursor = ytp_cursor_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *anns = ytp_anns_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  ytp_subs_t subs;
  ytp_subs_init(&subs, yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto stream11 =
      ytp_anns_stream(anns, 5, "peer1", 3, "ch1", 9, "encoding1", &error);
  ASSERT_EQ(error, nullptr);

  auto stream12 =
      ytp_anns_stream(anns, 5, "peer1", 3, "ch2", 9, "encoding2", &error);
  ASSERT_EQ(error, nullptr);

  auto stream21 =
      ytp_anns_stream(anns, 5, "peer2", 3, "ch1", 9, "encoding3", &error);
  ASSERT_EQ(error, nullptr);

  {
    auto ptr = ytp_stream_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0000", 4);
    ytp_stream_commit(yamal, 5005, stream11, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }
  {
    auto ptr = ytp_stream_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0001", 4);
    ytp_stream_commit(yamal, 5006, stream21, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }
  {
    auto ptr = ytp_stream_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0002", 4);
    ytp_stream_commit(yamal, 5007, stream12, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }

  auto stream22 =
      ytp_anns_stream(anns, 5, "peer2", 3, "ch2", 9, "encoding4", &error);
  ASSERT_EQ(error, nullptr);

  {
    auto ptr = ytp_stream_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0003", 4);
    ytp_stream_commit(yamal, 5009, stream22, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }

  ytp_anns_stream(anns, 5, "peer1", 3, "ch2", 18, "encoding2_override", &error);
  ASSERT_NE(error, nullptr);

  auto stream12_redef =
      ytp_anns_stream(anns, 5, "peer1", 3, "ch2", 9, "encoding2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(stream12, stream12_redef);

  using MsgAnn =
      std::tuple<std::string_view, std::string_view, std::string_view>;
  using MsgSub = uint64_t;
  struct MsgIdx : std::tuple<ytp_stream_t, uint64_t, std::string_view> {
    using tuple::tuple;
  };
  using MsgData = std::tuple<uint64_t, ytp_stream_t, std::string_view>;
  using Msg = std::variant<std::monostate, MsgAnn, MsgSub, MsgIdx, MsgData>;

  auto publish = [&](const Msg &msg) {
    return std::visit(
        fmc::overloaded{
            [&](const MsgAnn &msg) {
              auto &[p, c, e] = msg;
              auto it =
                  ytp_stream_write_ann(yamal, p.size(), p.data(), c.size(),
                                       c.data(), e.size(), e.data(), &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [&](const MsgSub &stream) {
              auto it = ytp_stream_write_sub(yamal, stream, &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [&](const MsgIdx &msg) {
              auto &[stream, offdata, payload] =
                  (std::tuple<ytp_stream_t, uint64_t, std::string_view> &)msg;
              auto it =
                  ytp_stream_write_idx(yamal, stream, offdata, payload.size(),
                                       payload.data(), &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [&](const MsgData &msg) {
              auto &[msgtime, stream, data] = msg;
              auto ptr = ytp_stream_reserve(yamal, data.size(), &error);
              EXPECT_EQ(error, nullptr);
              std::memcpy(ptr, data.data(), data.size());
              auto it = ytp_stream_commit(yamal, msgtime, stream, ptr, &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [](std::monostate) { return uint64_t{}; }},
        msg);
  };

  {
    ytp_stream_t substream;
    std::vector<Msg> output;

    publish(MsgAnn{"peer1", "ch1", "encoding1"});
    publish(MsgAnn{"peer1", "ch1", "update-encoding123"});

    publish(MsgAnn{"peer1", "ch2", "encoding2"});
    publish(MsgAnn{"peer2", "ch1", "encoding3"});

    publish(MsgData{5009, stream11, tostr("0004")});
    publish(MsgData{5009, stream12, tostr("0005")});
    publish(MsgData{5009, stream21, tostr("0006")});
    publish(MsgSub(stream21));

    while (ytp_subs_next(&subs, &substream, &error)) {
      output.emplace_back(MsgSub(substream));
    }

    publish(MsgSub(stream22));

    {
      auto cb = helper.anncb(
          [&](ytp_stream_t stream, uint64_t seqno, size_t peer_sz,
              const char *peer_name, size_t ch_sz, const char *ch_name,
              size_t encoding_sz, const char *encoding_data) {
            auto peername = std::string_view(peer_name, peer_sz);
            auto chname = std::string_view(ch_name, ch_sz);
            auto encoding = std::string_view(encoding_data, encoding_sz);

            output.push_back(MsgAnn(peername, chname, encoding));

            if (peername != "peer1" || chname != "ch1") {
              auto cb = helper.datacb([&](uint64_t seqno, uint64_t msgtime,
                                          ytp_stream_t stream, size_t sz,
                                          const char *data) {
                output.push_back(MsgData(msgtime, stream, {data, sz}));
              });
              ytp_cursor_data_cb(cursor, stream, cb.first, cb.second, &error);
              EXPECT_EQ(error, nullptr);
            }
          });
      ytp_cursor_ann_cb(cursor, cb.first, cb.second, &error);
      ASSERT_EQ(error, nullptr);
    }

    while (ytp_cursor_poll(cursor, &error)) {
      ASSERT_EQ(error, nullptr);
    }
    ASSERT_EQ(error, nullptr);

    while (ytp_subs_next(&subs, &substream, &error)) {
      output.emplace_back(MsgSub(substream));
    }

    auto readone = [&output, p = output.begin()]() mutable -> Msg {
      if (p == output.end()) {
        return std::monostate{};
      }
      return *(p++);
    };

    EXPECT_EQ(readone(), Msg{MsgSub(stream21)});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer1", "ch1", "encoding1")});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer1", "ch2", "encoding2")});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch1", "encoding3")});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch2", "encoding4")});
    EXPECT_EQ(readone(), Msg{MsgData(5006, stream21, "0001")});
    EXPECT_EQ(readone(), Msg{MsgData(5007, stream12, "0002")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream22, "0003")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream12, "0005")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream21, "0006")});
    EXPECT_EQ(readone(), Msg{MsgSub(stream12)});
    EXPECT_EQ(readone(), Msg{MsgSub(stream22)});
    EXPECT_EQ(readone(), Msg{std::monostate{}});

    ytp_iterator_t it[4];
    for (size_t i = 0; i < 4; ++i) {
      it[i] = ytp_yamal_begin(yamal, i, &error);
      ASSERT_EQ(error, nullptr);
    }

    auto readoneraw = [&](size_t idx) -> std::string_view {
      if (ytp_yamal_term(it[idx])) {
        return {};
      }

      uint64_t seqno;
      size_t sz;
      const char *data;
      ytp_yamal_read(yamal, it[idx], &seqno, &sz, &data, &error);
      EXPECT_EQ(error, nullptr);

      it[idx] = ytp_yamal_next(yamal, it[idx], &error);
      EXPECT_EQ(error, nullptr);

      return {data, sz};
    };
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5005}, stream11, "0000"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5006}, stream21, "0001"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5007}, stream12, "0002"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5009}, stream22, "0003"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5009}, stream11, "0004"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5009}, stream12, "0005"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA),
              buildmsg(uint64_t{5009}, stream21, "0006"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_DATA), "");

    std::vector<size_t> subs;
    subs.push_back(ytp_yamal_tell(yamal, it[YTP_STREAM_LIST_SUB], &error));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_SUB), buildmsg(stream21));
    subs.push_back(ytp_yamal_tell(yamal, it[YTP_STREAM_LIST_SUB], &error));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_SUB), buildmsg(stream22));
    subs.push_back(ytp_yamal_tell(yamal, it[YTP_STREAM_LIST_SUB], &error));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_SUB), buildmsg(stream12));
    subs.push_back(ytp_yamal_tell(yamal, it[YTP_STREAM_LIST_SUB], &error));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_SUB), buildmsg(stream22));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_SUB), "");

    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_NO_SUBSCRIPTION},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{subs[2]}, uint16_t{5}, uint16_t{3}, "peer1",
                       "ch2", "encoding2"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{subs[0]}, uint16_t{5}, uint16_t{3}, "peer2",
                       "ch1", "encoding3"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{subs[3]}, uint16_t{5}, uint16_t{3}, "peer2",
                       "ch2", "encoding4"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_UNKNOWN},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_UNKNOWN},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch1",
                       "update-encoding123"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_UNKNOWN},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_UNKNOWN},
                       uint16_t{5}, uint16_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN), "");

    for (size_t i = 0; i < 4; ++i) {
      it[i] = ytp_yamal_begin(yamal, i, &error);
      ASSERT_EQ(error, nullptr);
    }

    ytp_anns_stream(anns, 5, "peer2", 3, "ch3", 9, "encoding5", &error);
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_NO_SUBSCRIPTION},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{subs[2]}, uint16_t{5}, uint16_t{3}, "peer1",
                       "ch2", "encoding2"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{subs[0]}, uint16_t{5}, uint16_t{3}, "peer2",
                       "ch1", "encoding3"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{subs[3]}, uint16_t{5}, uint16_t{3}, "peer2",
                       "ch2", "encoding4"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_DUPLICATED},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_DUPLICATED},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch1",
                       "update-encoding123"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_DUPLICATED},
                       uint16_t{5}, uint16_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_DUPLICATED},
                       uint16_t{5}, uint16_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN),
              buildmsg(uint64_t{SUBSCRIPTION_STATE::YTP_SUB_NO_SUBSCRIPTION},
                       uint16_t{5}, uint16_t{3}, "peer2", "ch3", "encoding5"));
    EXPECT_EQ(readoneraw(YTP_STREAM_LIST_ANN), "");

    EXPECT_EQ(readone(), Msg{std::monostate{}});

    while (ytp_cursor_poll(cursor, &error)) {
      ASSERT_EQ(error, nullptr);
    }
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch3", "encoding5")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});
  }

  ytp_anns_del(anns, &error);
  ASSERT_EQ(error, nullptr);

  ytp_cursor_del(cursor, &error);
  ASSERT_EQ(error, nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
