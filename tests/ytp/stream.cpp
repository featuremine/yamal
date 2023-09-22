/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file stream.cpp
 * @date 8 May 2023
 * @brief File contains tests for YTP stream API
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp++/yamal.hpp>
#include <ytp/cursor.h>
#include <ytp/data.h>
#include <ytp/glob.h>
#include <ytp/index.h>
#include <ytp/stream.h>
#include <ytp/streams.h>
#include <ytp/subscription.h>
#include <ytp/yamal.h>

#include <fmc++/gtestwrap.hpp>
#include <fmc++/mpl.hpp>
#include <fmc/files.h>

#include <deque>
#include <list>

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
    return {
        &static_cb<F, uint64_t, int64_t, ytp_mmnode_offs, size_t, const char *>,
        cl};
  }

  template <typename F> std::pair<ytp_cursor_ann_cb_t, void *> anncb(F f) {
    auto *cl =
        callbacks
            .emplace_back(std::make_unique<closure_wrapper<F>>(std::move(f)))
            .get();
    return {&static_cb<F, uint64_t, ytp_mmnode_offs, size_t, const char *,
                       size_t, const char *, size_t, const char *, bool>,
            cl};
  }

  std::list<std::unique_ptr<closure_wrapper_base>> callbacks;
};

using MsgAnn = std::tuple<std::string_view, std::string_view, std::string_view>;
using MsgSub = uint64_t;
struct MsgIdx : std::tuple<ytp_mmnode_offs, ytp_mmnode_offs, std::string_view> {
  using tuple::tuple;
};
using MsgData = std::tuple<int64_t, ytp_mmnode_offs, std::string_view>;
using Msg = std::variant<std::monostate, MsgAnn, MsgSub, MsgIdx, MsgData>;

struct publisher_t {
  ytp_mmnode_offs operator()(const Msg &msg) {
    return std::visit(
        fmc::overloaded{
            [&](const MsgAnn &msg) {
              auto &[p, c, e] = msg;
              auto it =
                  ytp_announcement_write(yamal, p.size(), p.data(), c.size(),
                                         c.data(), e.size(), e.data(), &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [&](const MsgSub &stream) {
              auto it = ytp_subscription_write(yamal, stream, &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [&](const MsgIdx &msg) {
              auto &[stream, offdata, payload] =
                  static_cast<const std::tuple<ytp_mmnode_offs, ytp_mmnode_offs,
                                               std::string_view> &>(msg);
              auto it = ytp_index_write(yamal, stream, offdata, payload.size(),
                                        payload.data(), &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [&](const MsgData &msg) {
              auto &[msgtime, stream, data] = msg;
              auto ptr = ytp_data_reserve(yamal, data.size(), &error);
              EXPECT_EQ(error, nullptr);
              std::memcpy(ptr, data.data(), data.size());
              auto it = ytp_data_commit(yamal, msgtime, stream, ptr, &error);
              EXPECT_EQ(error, nullptr);
              auto off = ytp_yamal_tell(yamal, it, &error);
              EXPECT_EQ(error, nullptr);
              return off;
            },
            [](std::monostate) { return uint64_t{}; }},
        msg);
  }
  ytp_yamal_t *yamal;
  fmc_error_t *&error;
};

struct readoneraw_t {
  readoneraw_t(ytp_yamal_t *yamal, fmc_error_t *&error, size_t idx)
      : yamal(yamal), error(error), idx(idx),
        it(ytp_yamal_begin(yamal, idx, &error)) {}

  std::string_view operator()() {
    if (ytp_yamal_term(it)) {
      return {};
    }

    uint64_t seqno;
    size_t sz;
    const char *data;
    ytp_yamal_read(yamal, it, &seqno, &sz, &data, &error);
    EXPECT_EQ(error, nullptr);

    it = ytp_yamal_next(yamal, it, &error);
    EXPECT_EQ(error, nullptr);

    return {data, sz};
  }
  void restart() {
    it = ytp_yamal_begin(yamal, idx, &error);
    EXPECT_EQ(error, nullptr);
  }
  void seek(ytp_mmnode_offs offset) {
    it = ytp_yamal_seek(yamal, offset, &error);
    EXPECT_EQ(error, nullptr);
  }
  ytp_yamal_t *yamal;
  fmc_error_t *&error;
  size_t idx;
  ytp_iterator_t it;
};

template <typename T> struct readone_t {
  readone_t(ytp_yamal_t *yamal, fmc_error_t *&error, std::list<T> &list)
      : yamal(yamal), error(error), list(list) {}

  T operator()() {
    if (list.empty()) {
      return {};
    }

    auto it = list.begin();
    T ret = std::move(*it);
    list.erase(it);
    return ret;
  }

  ytp_yamal_t *yamal;
  fmc_error_t *&error;
  std::list<T> &list;
};

TEST(stream, main_test_1) {
  callback_helper helper;

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  publisher_t publish{yamal, error};
  readoneraw_t readoneraw_data(yamal, error, YTP_STREAM_LIST_DATA);
  readoneraw_t readoneraw_anns(yamal, error, YTP_STREAM_LIST_ANNS);
  readoneraw_t readoneraw_subs(yamal, error, YTP_STREAM_LIST_SUBS);
  readoneraw_t readoneraw_indx(yamal, error, YTP_STREAM_LIST_INDX);

  auto *cursor = ytp_cursor_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *anns = ytp_streams_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto subs_it = ytp_subscription_begin(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto stream11 =
      ytp_streams_announce(anns, 5, "peer1", 3, "ch1", 9, "encoding1", &error);
  ASSERT_EQ(error, nullptr);

  auto stream12 =
      ytp_streams_announce(anns, 5, "peer1", 3, "ch2", 9, "encoding2", &error);
  ASSERT_EQ(error, nullptr);

  auto stream21 =
      ytp_streams_announce(anns, 5, "peer2", 3, "ch1", 9, "encoding3", &error);
  ASSERT_EQ(error, nullptr);

  ytp_mmnode_offs msg0003;
  ytp_mmnode_offs msg0004;

  {
    auto ptr = ytp_data_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0000", 4);
    ytp_data_commit(yamal, 5005, stream11, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }
  {
    auto ptr = ytp_data_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0001", 4);
    ytp_data_commit(yamal, 5006, stream21, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }
  {
    auto ptr = ytp_data_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0002", 4);
    ytp_data_commit(yamal, 5007, stream12, ptr, &error);
    ASSERT_EQ(error, nullptr);
  }

  auto stream22 =
      ytp_streams_announce(anns, 5, "peer2", 3, "ch2", 9, "encoding4", &error);
  ASSERT_EQ(error, nullptr);

  {
    auto ptr = ytp_data_reserve(yamal, 4, &error);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr, "0003", 4);
    auto it = ytp_data_commit(yamal, 5009, stream22, ptr, &error);
    ASSERT_EQ(error, nullptr);
    msg0003 = ytp_yamal_tell(yamal, it, &error);
  }

  ytp_streams_announce(anns, 5, "peer1", 3, "ch2", 18, "encoding2_override",
                       &error);
  ASSERT_NE(error, nullptr);

  auto stream12_redef =
      ytp_streams_announce(anns, 5, "peer1", 3, "ch2", 9, "encoding2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(stream12, stream12_redef);

  std::list<Msg> output;
  {
    ytp_mmnode_offs substream;
    readone_t readone(yamal, error, output);

    publish(MsgAnn{"peer1", "ch1", "encoding1"});
    publish(MsgAnn{"peer1", "ch1", "update-encoding123"});

    publish(MsgAnn{"peer1", "ch2", "encoding2"});
    publish(MsgAnn{"peer2", "ch1", "encoding3"});

    msg0004 = publish(MsgData{5009, stream11, tostr("0004")});
    publish(MsgData{5009, stream12, tostr("0005")});
    publish(MsgData{5009, stream21, tostr("0006")});
    auto sub21 = publish(MsgSub(stream21));

    while (ytp_subscription_next(yamal, &subs_it, &substream, &error)) {
      output.emplace_back(MsgSub(substream));
    }
    ASSERT_EQ(error, nullptr);

    auto sub22 = publish(MsgSub(stream22));

    {
      auto cb = helper.anncb(
          [&](uint64_t seqno, ytp_mmnode_offs stream, size_t peer_sz,
              const char *peer_name, size_t ch_sz, const char *ch_name,
              size_t encoding_sz, const char *encoding_data, bool subscribed) {
            auto peername = std::string_view(peer_name, peer_sz);
            auto chname = std::string_view(ch_name, ch_sz);
            auto encoding = std::string_view(encoding_data, encoding_sz);

            output.push_back(MsgAnn(peername, chname, encoding));

            if (peername != "peer1" || chname != "ch1") {
              auto cb = helper.datacb([&](uint64_t seqno, int64_t msgtime,
                                          ytp_mmnode_offs stream, size_t sz,
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

    while (ytp_subscription_next(yamal, &subs_it, &substream, &error)) {
      output.emplace_back(MsgSub(substream));
    }

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

    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5005}, stream11, "0000"));
    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5006}, stream21, "0001"));
    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5007}, stream12, "0002"));
    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5009}, stream22, "0003"));
    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5009}, stream11, "0004"));
    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5009}, stream12, "0005"));
    EXPECT_EQ(readoneraw_data(), buildmsg(uint64_t{5009}, stream21, "0006"));
    EXPECT_EQ(readoneraw_data(), "");

    std::vector<size_t> subs;
    subs.push_back(ytp_yamal_tell(yamal, readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream21));
    subs.push_back(ytp_yamal_tell(yamal, readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream22));
    subs.push_back(ytp_yamal_tell(yamal, readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream12));
    subs.push_back(ytp_yamal_tell(yamal, readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream22));
    EXPECT_EQ(readoneraw_subs(), "");

    EXPECT_EQ(subs[0], sub21);
    EXPECT_EQ(subs[1], sub22);
    EXPECT_EQ(subs[2], 1616);
    EXPECT_EQ(subs[3], 1656);

    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream12}, uint64_t{subs[2]}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream21}, uint64_t{sub21}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream22}, uint64_t{subs[3]}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch2", "encoding4"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer1", "ch1", "update-encoding123"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(), "");

    readoneraw_data.restart();
    readoneraw_anns.restart();
    readoneraw_subs.restart();
    readoneraw_indx.restart();

    auto stream23 = ytp_streams_announce(anns, 5, "peer2", 3, "ch3", 9,
                                         "encoding5", &error);
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream12}, uint64_t{subs[2]}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream21}, uint64_t{subs[0]}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream22}, uint64_t{subs[3]}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch2", "encoding4"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "update-encoding123"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream12}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream21}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream23}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch3", "encoding5"));
    EXPECT_EQ(readoneraw_anns(), "");

    EXPECT_EQ(readone(), Msg{std::monostate{}});

    while (ytp_cursor_poll(cursor, &error)) {
      ASSERT_EQ(error, nullptr);
    }
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch3", "encoding5")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});
  }

  {
    publish(MsgIdx(stream22, msg0003, "index22_1"));
    publish(MsgIdx(stream11, msg0004, "index11_1"));

    readone_t readone(yamal, error, output);

    auto idx_it = ytp_index_begin(yamal, &error);
    ASSERT_EQ(error, nullptr);

    while (!ytp_yamal_term(idx_it)) {
      uint64_t seqno;
      ytp_mmnode_offs stream;
      ytp_mmnode_offs offset;
      size_t sz;
      const char *payload;
      ytp_index_read(yamal, idx_it, &seqno, &stream, &offset, &sz, &payload,
                     &error);
      ASSERT_EQ(error, nullptr);

      output.emplace_back(
          MsgIdx(stream, offset, std::string_view(payload, sz)));

      idx_it = ytp_yamal_next(yamal, idx_it, &error);
      ASSERT_EQ(error, nullptr);
    }

    EXPECT_EQ(readone(), Msg{MsgIdx(stream22, msg0003, "index22_1")});
    EXPECT_EQ(readone(), Msg{MsgIdx(stream11, msg0004, "index11_1")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});

    ytp_cursor_seek(cursor, msg0003, &error);
    ASSERT_EQ(error, nullptr);

    while (ytp_cursor_poll(cursor, &error)) {
      ASSERT_EQ(error, nullptr);
    }
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readone(), Msg{MsgData(5009, stream22, "0003")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream12, "0005")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream21, "0006")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});
  }

  ytp_streams_del(anns, &error);
  ASSERT_EQ(error, nullptr);

  ytp_cursor_del(cursor, &error);
  ASSERT_EQ(error, nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(stream, cursor_consume) {
  callback_helper helper;

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *anns = ytp_streams_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  publisher_t publish{yamal, error};
  readoneraw_t readoneraw_data(yamal, error, YTP_STREAM_LIST_DATA);
  readoneraw_t readoneraw_anns(yamal, error, YTP_STREAM_LIST_ANNS);
  readoneraw_t readoneraw_subs(yamal, error, YTP_STREAM_LIST_SUBS);
  readoneraw_t readoneraw_indx(yamal, error, YTP_STREAM_LIST_INDX);

  std::list<std::string> output_1;
  std::list<std::string> output_2;
  std::list<std::string> output_3;
  readone_t readone1(yamal, error, output_1);
  readone_t readone2(yamal, error, output_2);
  readone_t readone3(yamal, error, output_3);

  auto add_datacb = [&](std::string name, std::list<std::string> &output,
                        ytp_cursor_t *cursor, ytp_mmnode_offs stream) {
    auto cb =
        helper.datacb([&, name = std::move(name)](
                          uint64_t seqno, int64_t msgtime,
                          ytp_mmnode_offs stream, size_t sz, const char *data) {
          std::string s;
          s += name;
          s += ": ";
          s += std::string_view{data, sz};
          output.emplace_back(std::move(s));
        });
    ytp_cursor_data_cb(cursor, stream, cb.first, cb.second, &error);
    EXPECT_EQ(error, nullptr);
  };

  auto add_prfxcb = [&](std::string name, std::list<std::string> &output,
                        ytp_glob_t *glob, std::string_view prfx) {
    auto cb =
        helper.datacb([&, prfx, name = std::move(name)](
                          uint64_t seqno, int64_t msgtime,
                          ytp_mmnode_offs stream, size_t sz, const char *data) {
          std::string s;
          s += name;
          s += " ";
          s += prfx;
          s += ": ";
          s += std::string_view{data, sz};
          output.emplace_back(std::move(s));
        });
    ytp_glob_prefix_cb(glob, prfx.size(), prfx.data(), cb.first, cb.second,
                       &error);
    EXPECT_EQ(error, nullptr);
  };

  auto add_annscb = [&](std::string name, std::list<std::string> &output,
                        ytp_cursor_t *cursor) {
    auto cb = helper.anncb(
        [&, name = std::move(name)](
            uint64_t seqno, ytp_mmnode_offs stream, size_t peer_sz,
            const char *peer_name, size_t ch_sz, const char *ch_name,
            size_t encoding_sz, const char *encoding_data, bool subscribed) {
          std::string s;
          s += name;
          s += ": ";
          s += std::string_view{peer_name, peer_sz};
          s += ", ";
          s += std::string_view{ch_name, ch_sz};
          output.emplace_back(std::move(s));
        });
    ytp_cursor_ann_cb(cursor, cb.first, cb.second, &error);
    EXPECT_EQ(error, nullptr);
  };

  auto *cursor_1 = ytp_cursor_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *cursor_2 = ytp_cursor_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *cursor_3 = ytp_cursor_new(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *glob_1 = ytp_glob_new(cursor_1, &error);
  ASSERT_EQ(error, nullptr);

  auto *glob_2 = ytp_glob_new(cursor_2, &error);
  ASSERT_EQ(error, nullptr);

  auto *glob_3 = ytp_glob_new(cursor_3, &error);
  ASSERT_EQ(error, nullptr);

  auto stream11 =
      ytp_streams_announce(anns, 5, "peer1", 3, "ch1", 3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  auto stream12 =
      ytp_streams_announce(anns, 5, "peer1", 3, "ch2", 3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  auto stream23 =
      ytp_streams_announce(anns, 5, "peer2", 3, "ch3", 3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  auto stream24 =
      ytp_streams_announce(anns, 5, "peer2", 3, "ch4", 3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  publish(MsgData{5009, stream11, tostr("0001")});
  publish(MsgData{5009, stream23, tostr("0002")});
  publish(MsgData{5009, stream23, tostr("0003")});
  publish(MsgData{5009, stream11, tostr("0004")});
  publish(MsgData{5009, stream24, tostr("0005")});

  add_annscb("cb_01 cursor_1", output_1, cursor_1);
  add_annscb("cb_02 cursor_1", output_1, cursor_1);
  add_annscb("cb_03 cursor_2", output_2, cursor_2);
  add_annscb("cb_04 cursor_2", output_2, cursor_2);
  add_annscb("cb_05 cursor_3", output_3, cursor_3);
  add_annscb("cb_06 cursor_3", output_3, cursor_3);

  add_datacb("cb_07 cursor_2", output_2, cursor_2, stream23);
  add_datacb("cb_08 cursor_3", output_3, cursor_3, stream23);
  add_datacb("cb_09 cursor_3", output_3, cursor_3, stream11);
  add_datacb("cb_10 cursor_2", output_2, cursor_2, stream24);
  add_datacb("cb_11 cursor_1", output_1, cursor_1, stream12);
  add_datacb("cb_12 cursor_2", output_2, cursor_2, stream23);
  add_datacb("cb_13 cursor_3", output_3, cursor_3, stream11);
  add_datacb("cb_14 cursor_1", output_1, cursor_1, stream24);
  add_datacb("cb_15 cursor_1", output_1, cursor_1, stream23);

  add_prfxcb("cb_16 glob_1", output_1, glob_1, "prefix1/");
  add_prfxcb("cb_17 glob_1", output_1, glob_1, "prefix2/");
  add_prfxcb("cb_18 glob_2", output_2, glob_2, "prefix3/");
  add_prfxcb("cb_19 glob_3", output_3, glob_3, "/");

  for (int i = 0; i < 3; ++i) {
    auto ret = ytp_cursor_poll(cursor_3, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);
  }

  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer1, ch1");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer1, ch1");
  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer1, ch2");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer1, ch2");
  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer2, ch3");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer2, ch3");
  EXPECT_EQ(readone3(), "");

  for (int i = 0; i < 3; ++i) {
    auto ret = ytp_cursor_poll(cursor_3, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);
  }

  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer2, ch4");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer2, ch4");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0001");
  EXPECT_EQ(readone3(), "cb_13 cursor_3: 0001");
  EXPECT_EQ(readone3(), "cb_09 cursor_3: 0001");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0002");
  EXPECT_EQ(readone3(), "cb_08 cursor_3: 0002");
  EXPECT_EQ(readone3(), "");

  for (int i = 0; i < 3; ++i) {
    auto ret = ytp_cursor_poll(cursor_2, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);
  }

  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer1, ch1");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer1, ch1");
  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer1, ch2");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer1, ch2");
  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer2, ch3");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer2, ch3");
  EXPECT_EQ(readone2(), "");

  {
    auto ret = ytp_cursor_consume(cursor_2, cursor_3, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_FALSE(ret);
  }

  {
    auto ret = ytp_cursor_consume(cursor_1, cursor_3, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_FALSE(ret);
  }

  for (int i = 0; i < 3; ++i) {
    auto ret = ytp_cursor_poll(cursor_2, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);
  }

  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer2, ch4");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer2, ch4");
  EXPECT_EQ(readone2(), "cb_12 cursor_2: 0002");
  EXPECT_EQ(readone2(), "cb_07 cursor_2: 0002");
  EXPECT_EQ(readone2(), "");

  {
    auto ret = ytp_cursor_consume(cursor_2, cursor_3, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);

    ytp_glob_consume(glob_2, glob_3, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto ret = ytp_cursor_consume(cursor_1, cursor_2, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_FALSE(ret);
  }

  while (ytp_cursor_poll(cursor_3, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  EXPECT_EQ(readone1(), "");
  EXPECT_EQ(readone2(), "");
  EXPECT_EQ(readone3(), "");

  for (int i = 0; i < 6; ++i) {
    auto ret = ytp_cursor_poll(cursor_1, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);
  }

  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer1, ch1");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer1, ch1");
  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer1, ch2");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer1, ch2");
  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer2, ch3");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer2, ch3");
  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer2, ch4");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer2, ch4");
  EXPECT_EQ(readone1(), "cb_15 cursor_1: 0002");
  EXPECT_EQ(readone1(), "");

  EXPECT_EQ(readone2(), "");
  EXPECT_EQ(readone3(), "");

  {
    auto ret = ytp_cursor_consume(cursor_1, cursor_2, &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_TRUE(ret);

    ytp_glob_consume(glob_1, glob_2, &error);
    ASSERT_EQ(error, nullptr);
  }

  while (ytp_cursor_poll(cursor_2, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  while (ytp_cursor_poll(cursor_3, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  EXPECT_EQ(readone1(), "");
  EXPECT_EQ(readone2(), "");
  EXPECT_EQ(readone3(), "");

  while (ytp_cursor_poll(cursor_1, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  EXPECT_EQ(readone1(), "cb_15 cursor_1: 0003");
  EXPECT_EQ(readone1(), "cb_14 cursor_1: 0005");
  EXPECT_EQ(readone1(), "");

  EXPECT_EQ(readone2(), "cb_12 cursor_2: 0003");
  EXPECT_EQ(readone2(), "cb_07 cursor_2: 0003");
  EXPECT_EQ(readone2(), "cb_10 cursor_2: 0005");
  EXPECT_EQ(readone2(), "");

  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0003");
  EXPECT_EQ(readone3(), "cb_08 cursor_3: 0003");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0004");
  EXPECT_EQ(readone3(), "cb_13 cursor_3: 0004");
  EXPECT_EQ(readone3(), "cb_09 cursor_3: 0004");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0005");
  EXPECT_EQ(readone3(), "");

  auto stream_prfx1 = ytp_streams_announce(anns, 5, "peer2", 11, "prefix1/a/b",
                                           3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  auto stream_prfx2 = ytp_streams_announce(anns, 5, "peer2", 11, "prefix2/a/b",
                                           3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  auto stream_prfx3 = ytp_streams_announce(anns, 5, "peer2", 11, "prefix3/a/b",
                                           3, "enc", &error);
  ASSERT_EQ(error, nullptr);

  publish(MsgData{5009, stream_prfx1, tostr("0006")});
  publish(MsgData{5009, stream_prfx2, tostr("0007")});
  publish(MsgData{5009, stream_prfx3, tostr("0008")});

  while (ytp_cursor_poll(cursor_2, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  while (ytp_cursor_poll(cursor_3, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  EXPECT_EQ(readone1(), "");
  EXPECT_EQ(readone2(), "");
  EXPECT_EQ(readone3(), "");

  while (ytp_cursor_poll(cursor_1, &error)) {
    ASSERT_EQ(error, nullptr);
  }
  ASSERT_EQ(error, nullptr);

  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer2, prefix1/a/b");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer2, prefix1/a/b");
  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer2, prefix2/a/b");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer2, prefix2/a/b");
  EXPECT_EQ(readone1(), "cb_02 cursor_1: peer2, prefix3/a/b");
  EXPECT_EQ(readone1(), "cb_01 cursor_1: peer2, prefix3/a/b");
  EXPECT_EQ(readone1(), "cb_16 glob_1 prefix1/: 0006");
  EXPECT_EQ(readone1(), "cb_17 glob_1 prefix2/: 0007");
  EXPECT_EQ(readone1(), "");

  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer2, prefix1/a/b");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer2, prefix1/a/b");
  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer2, prefix2/a/b");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer2, prefix2/a/b");
  EXPECT_EQ(readone2(), "cb_04 cursor_2: peer2, prefix3/a/b");
  EXPECT_EQ(readone2(), "cb_03 cursor_2: peer2, prefix3/a/b");
  EXPECT_EQ(readone2(), "cb_18 glob_2 prefix3/: 0008");
  EXPECT_EQ(readone2(), "");

  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer2, prefix1/a/b");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer2, prefix1/a/b");
  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer2, prefix2/a/b");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer2, prefix2/a/b");
  EXPECT_EQ(readone3(), "cb_06 cursor_3: peer2, prefix3/a/b");
  EXPECT_EQ(readone3(), "cb_05 cursor_3: peer2, prefix3/a/b");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0006");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0007");
  EXPECT_EQ(readone3(), "cb_19 glob_3 /: 0008");
  EXPECT_EQ(readone3(), "");

  ytp_cursor_del(cursor_3, &error);
  ASSERT_EQ(error, nullptr);

  ytp_cursor_del(cursor_2, &error);
  ASSERT_EQ(error, nullptr);

  ytp_cursor_del(cursor_1, &error);
  ASSERT_EQ(error, nullptr);

  ytp_streams_del(anns, &error);
  ASSERT_EQ(error, nullptr);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(stream, cpp_main_test_1) {
  callback_helper helper;

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  fmc::scope_end_call fdc([&]() {
    fmc_fclose(fd, &error);
    ASSERT_EQ(error, nullptr);
  });

  ytp::yamal yamal = ytp::yamal(fd);

  publisher_t publish{yamal.get(), error};
  readoneraw_t readoneraw_data(yamal.get(), error, YTP_STREAM_LIST_DATA);
  readoneraw_t readoneraw_anns(yamal.get(), error, YTP_STREAM_LIST_ANNS);
  readoneraw_t readoneraw_subs(yamal.get(), error, YTP_STREAM_LIST_SUBS);
  readoneraw_t readoneraw_indx(yamal.get(), error, YTP_STREAM_LIST_INDX);

  auto *cursor = ytp_cursor_new(yamal.get(), &error);
  ASSERT_EQ(error, nullptr);

  auto anns = yamal.streams();

  auto subs_it = ytp_subscription_begin(yamal.get(), &error);
  ASSERT_EQ(error, nullptr);

  auto stream11 = anns.announce("peer1", "ch1", "encoding1");
  auto stream12 = anns.announce("peer1", "ch2", "encoding2");
  auto stream21 = anns.announce("peer2", "ch1", "encoding3");

  ytp_mmnode_offs msg0003;
  ytp_mmnode_offs msg0004;

  ytp::data data = yamal.data();

  {
    auto ptr = data.reserve(4);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr.data(), "0000", 4);
    data.commit(5005, stream11, ptr);
    ASSERT_EQ(error, nullptr);
  }
  {
    auto ptr = data.reserve(4);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr.data(), "0001", 4);
    data.commit(5006, stream21, ptr);
    ASSERT_EQ(error, nullptr);
  }
  {
    auto ptr = data.reserve(4);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr.data(), "0002", 4);
    data.commit(5007, stream12, ptr);
    ASSERT_EQ(error, nullptr);
  }

  auto stream22 = anns.announce("peer2", "ch2", "encoding4");
  ASSERT_EQ(error, nullptr);

  {
    auto ptr = data.reserve(4);
    ASSERT_EQ(error, nullptr);
    std::memcpy(ptr.data(), "0003", 4);
    auto it = data.commit(5009, stream22, ptr);
    ASSERT_EQ(error, nullptr);
    msg0003 = ytp_yamal_tell(yamal.get(), it, &error);
  }

  ASSERT_THROW(anns.announce("peer1", "ch2", "encoding2_override"),
               std::runtime_error);

  auto stream12_redef = anns.announce("peer1", "ch2", "encoding2");
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(stream12, stream12_redef);

  std::list<Msg> output;
  {
    ytp_mmnode_offs substream;
    readone_t readone(yamal.get(), error, output);

    publish(MsgAnn{"peer1", "ch1", "encoding1"});
    publish(MsgAnn{"peer1", "ch1", "update-encoding123"});

    publish(MsgAnn{"peer1", "ch2", "encoding2"});
    publish(MsgAnn{"peer2", "ch1", "encoding3"});

    msg0004 = publish(MsgData{5009, stream11.id(), tostr("0004")});
    publish(MsgData{5009, stream12.id(), tostr("0005")});
    publish(MsgData{5009, stream21.id(), tostr("0006")});
    auto sub21 = publish(MsgSub(stream21.id()));

    while (ytp_subscription_next(yamal.get(), &subs_it, &substream, &error)) {
      output.emplace_back(MsgSub(substream));
    }
    ASSERT_EQ(error, nullptr);

    auto sub22 = publish(MsgSub(stream22.id()));

    {
      auto cb = helper.anncb(
          [&](uint64_t seqno, ytp_mmnode_offs stream, size_t peer_sz,
              const char *peer_name, size_t ch_sz, const char *ch_name,
              size_t encoding_sz, const char *encoding_data, bool subscribed) {
            auto peername = std::string_view(peer_name, peer_sz);
            auto chname = std::string_view(ch_name, ch_sz);
            auto encoding = std::string_view(encoding_data, encoding_sz);

            output.push_back(MsgAnn(peername, chname, encoding));

            if (peername != "peer1" || chname != "ch1") {
              auto cb = helper.datacb([&](uint64_t seqno, int64_t msgtime,
                                          ytp_mmnode_offs stream, size_t sz,
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

    while (ytp_subscription_next(yamal.get(), &subs_it, &substream, &error)) {
      output.emplace_back(MsgSub(substream));
    }

    EXPECT_EQ(readone(), Msg{MsgSub(stream21.id())});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer1", "ch1", "encoding1")});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer1", "ch2", "encoding2")});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch1", "encoding3")});
    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch2", "encoding4")});
    EXPECT_EQ(readone(), Msg{MsgData(5006, stream21.id(), "0001")});
    EXPECT_EQ(readone(), Msg{MsgData(5007, stream12.id(), "0002")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream22.id(), "0003")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream12.id(), "0005")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream21.id(), "0006")});
    EXPECT_EQ(readone(), Msg{MsgSub(stream12.id())});
    EXPECT_EQ(readone(), Msg{MsgSub(stream22.id())});
    EXPECT_EQ(readone(), Msg{std::monostate{}});

    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5005}, stream11.id(), "0000"));
    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5006}, stream21.id(), "0001"));
    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5007}, stream12.id(), "0002"));
    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5009}, stream22.id(), "0003"));
    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5009}, stream11.id(), "0004"));
    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5009}, stream12.id(), "0005"));
    EXPECT_EQ(readoneraw_data(),
              buildmsg(uint64_t{5009}, stream21.id(), "0006"));
    EXPECT_EQ(readoneraw_data(), "");

    std::vector<size_t> subs;
    subs.push_back(ytp_yamal_tell(yamal.get(), readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream21.id()));
    subs.push_back(ytp_yamal_tell(yamal.get(), readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream22.id()));
    subs.push_back(ytp_yamal_tell(yamal.get(), readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream12.id()));
    subs.push_back(ytp_yamal_tell(yamal.get(), readoneraw_subs.it, &error));
    EXPECT_EQ(readoneraw_subs(), buildmsg(stream22.id()));
    EXPECT_EQ(readoneraw_subs(), "");

    EXPECT_EQ(subs[0], sub21);
    EXPECT_EQ(subs[1], sub22);
    EXPECT_EQ(subs[2], 1616);
    EXPECT_EQ(subs[3], 1656);

    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream12.id()}, uint64_t{subs[2]}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream21.id()}, uint64_t{sub21}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream22.id()}, uint64_t{subs[3]}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch2", "encoding4"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer1", "ch1", "update-encoding123"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{0}, uint64_t{0}, uint32_t{5}, uint32_t{3},
                       "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(), "");

    readoneraw_data.restart();
    readoneraw_anns.restart();
    readoneraw_subs.restart();
    readoneraw_indx.restart();

    auto stream23 = anns.announce("peer2", "ch3", "encoding5");

    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream12.id()}, uint64_t{subs[2]}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream21.id()}, uint64_t{subs[0]}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream22.id()}, uint64_t{subs[3]}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch2", "encoding4"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "encoding1"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream11.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch1", "update-encoding123"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream12.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer1", "ch2", "encoding2"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream21.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch1", "encoding3"));
    EXPECT_EQ(readoneraw_anns(),
              buildmsg(uint64_t{stream23.id()}, uint64_t{0}, uint32_t{5},
                       uint32_t{3}, "peer2", "ch3", "encoding5"));
    EXPECT_EQ(readoneraw_anns(), "");

    EXPECT_EQ(readone(), Msg{std::monostate{}});

    while (ytp_cursor_poll(cursor, &error)) {
      ASSERT_EQ(error, nullptr);
    }
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readone(), Msg{MsgAnn("peer2", "ch3", "encoding5")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});
  }

  {
    publish(MsgIdx(stream22.id(), msg0003, "index22_1"));
    publish(MsgIdx(stream11.id(), msg0004, "index11_1"));

    readone_t readone(yamal.get(), error, output);

    auto idx_it = ytp_index_begin(yamal.get(), &error);
    ASSERT_EQ(error, nullptr);

    while (!ytp_yamal_term(idx_it)) {
      uint64_t seqno;
      ytp_mmnode_offs stream;
      ytp_mmnode_offs offset;
      size_t sz;
      const char *payload;
      ytp_index_read(yamal.get(), idx_it, &seqno, &stream, &offset, &sz,
                     &payload, &error);
      ASSERT_EQ(error, nullptr);

      output.emplace_back(
          MsgIdx(stream, offset, std::string_view(payload, sz)));

      idx_it = ytp_yamal_next(yamal.get(), idx_it, &error);
      ASSERT_EQ(error, nullptr);
    }

    EXPECT_EQ(readone(), Msg{MsgIdx(stream22.id(), msg0003, "index22_1")});
    EXPECT_EQ(readone(), Msg{MsgIdx(stream11.id(), msg0004, "index11_1")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});

    ytp_cursor_seek(cursor, msg0003, &error);
    ASSERT_EQ(error, nullptr);

    while (ytp_cursor_poll(cursor, &error)) {
      ASSERT_EQ(error, nullptr);
    }
    ASSERT_EQ(error, nullptr);

    EXPECT_EQ(readone(), Msg{MsgData(5009, stream22.id(), "0003")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream12.id(), "0005")});
    EXPECT_EQ(readone(), Msg{MsgData(5009, stream21.id(), "0006")});
    EXPECT_EQ(readone(), Msg{std::monostate{}});
  }

  ytp_cursor_del(cursor, &error);
  ASSERT_EQ(error, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
