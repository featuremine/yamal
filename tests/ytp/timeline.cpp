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
 * @file timeline.cpp
 * @date 10 Jan 2022
 * @brief File contains tests for YTP timeline API
 *
 * @see http://www.featuremine.com
 */

#include <thread>

#include <ytp/control.h>
#include <ytp/time.h>
#include <ytp/timeline.h>
#include <ytp/yamal.h>

#include <fmc++/fs.hpp>
#include <fmc++/gtestwrap.hpp>
#include <fmc/files.h>

#include <list>

using namespace std;

template <std::size_t N> std::string_view tostr(const char (&str)[N]) {
  return {str, N - 1};
}

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

  template <typename F> std::pair<ytp_timeline_peer_cb_t, void *> peercb(F f) {
    auto *cl =
        callbacks
            .emplace_back(std::make_unique<closure_wrapper<F>>(std::move(f)))
            .get();
    return {&static_cb<F, ytp_peer_t, size_t, const char *>, cl};
  }

  template <typename F> std::pair<ytp_timeline_ch_cb_t, void *> chcb(F f) {
    auto *cl =
        callbacks
            .emplace_back(std::make_unique<closure_wrapper<F>>(std::move(f)))
            .get();
    return {&static_cb<F, ytp_peer_t, ytp_channel_t, uint64_t, size_t,
                       const char *>,
            cl};
  }

  template <typename F> std::pair<ytp_timeline_data_cb_t, void *> datacb(F f) {
    auto *cl =
        callbacks
            .emplace_back(std::make_unique<closure_wrapper<F>>(std::move(f)))
            .get();
    return {&static_cb<F, ytp_peer_t, ytp_channel_t, uint64_t, size_t,
                       const char *>,
            cl};
  }

  std::list<std::unique_ptr<closure_wrapper_base>> callbacks;
};

TEST(timeline, timeline8) {
  callback_helper helper;

  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);
  {
    auto peer1 = ytp_control_peer_decl(ctrl, 5, "peer1", &error);
    ASSERT_EQ(error, nullptr);

    auto peer2 = ytp_control_peer_decl(ctrl, 5, "peer2", &error);
    ASSERT_EQ(error, nullptr);

    ytp_channel_t ch1;
    ytp_channel_t ch2;

    ch1 = ytp_control_stream_decl(ctrl, 5001, peer1, 3, "ch1", 9, "encoding1",
                                  &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(ch1, YTP_CHANNEL_OFF);

    ch2 = ytp_control_stream_decl(ctrl, 5002, peer1, 3, "ch2", 9, "encoding2",
                                  &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(ch2, YTP_CHANNEL_OFF + 1);

    ch1 = ytp_control_stream_decl(ctrl, 5003, peer2, 3, "ch1", 9, "encoding3",
                                  &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(ch1, YTP_CHANNEL_OFF);

    {
      auto ptr = ytp_control_reserve(ctrl, 4, &error);
      ASSERT_EQ(error, nullptr);
      std::memcpy(ptr, "0000", 4);
      ytp_control_commit(ctrl, peer1, ch1, 5005, ptr, &error);
      ASSERT_EQ(error, nullptr);
    }
    {
      auto ptr = ytp_control_reserve(ctrl, 4, &error);
      ASSERT_EQ(error, nullptr);
      std::memcpy(ptr, "0001", 4);
      ytp_control_commit(ctrl, peer2, ch1, 5006, ptr, &error);
      ASSERT_EQ(error, nullptr);
    }
    {
      auto ptr = ytp_control_reserve(ctrl, 4, &error);
      ASSERT_EQ(error, nullptr);
      std::memcpy(ptr, "0002", 4);
      ytp_control_commit(ctrl, peer1, ch2, 5007, ptr, &error);
      ASSERT_EQ(error, nullptr);
    }

    ch2 = ytp_control_stream_decl(ctrl, 5008, peer2, 3, "ch2", 9, "encoding4",
                                  &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(ch2, YTP_CHANNEL_OFF + 1);

    {
      auto ptr = ytp_control_reserve(ctrl, 4, &error);
      ASSERT_EQ(error, nullptr);
      std::memcpy(ptr, "0003", 4);
      ytp_control_commit(ctrl, peer2, ch2, 5009, ptr, &error);
      ASSERT_EQ(error, nullptr);
    }

    ytp_control_stream_decl(ctrl, 5010, peer1, 3, "ch2", 18,
                            "encoding2_override", &error);
    ASSERT_NE(error, nullptr);

    ch2 = ytp_control_stream_decl(ctrl, 5010, peer1, 3, "ch2", 9, "encoding2",
                                  &error);
    ASSERT_EQ(error, nullptr);
    ASSERT_EQ(ch2, YTP_CHANNEL_OFF + 1);

    ytp_control_sub(ctrl, 5011, peer1, ch1, &error);
    ASSERT_EQ(error, nullptr);
    ytp_control_sub(ctrl, 5012, peer2, ch2, &error);
    ASSERT_EQ(error, nullptr);
    ytp_control_sub(ctrl, 5011, peer1, ch1, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto yamal = ytp_yamal_new(fd, &error);
    ASSERT_EQ(error, nullptr);

    using T = std::tuple<ytp_peer_t, ytp_channel_t, uint64_t, std::string_view>;
    auto publish = [&](const T &msg) {
      if (std::get<0>(msg) == YTP_PEER_ANN) {
        ytp_control_peer_decl(ctrl, std::get<3>(msg).size(),
                              std::get<3>(msg).data(), &error);
      } else {
        auto ptr = ytp_control_reserve(ctrl, std::get<3>(msg).size(), &error);
        ASSERT_EQ(error, nullptr);
        std::memcpy(ptr, std::get<3>(msg).data(), std::get<3>(msg).size());
        ytp_control_commit(ctrl, std::get<0>(msg), std::get<1>(msg),
                           std::get<2>(msg), ptr, &error);
      }
      ASSERT_EQ(error, nullptr);
    };

    publish({0, 0, 0, tostr("peer3")});
    publish({0, 0, 0, tostr("peer2")});
    publish({256, YTP_CHANNEL_ANN, 5001, tostr("\x3\0ch1encoding1")});
    publish({258, YTP_CHANNEL_ANN, 5001, tostr("\x3\0ch1encoding1")});
    publish({256, 256, 5009, tostr("0004")});
    publish({256, 257, 5009, tostr("0005")});
    publish({258, 256, 5009, tostr("0006")});

    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }

  {
    auto *timeline = ytp_timeline_new(ctrl, &error);
    ASSERT_EQ(error, nullptr);

    using P = std::tuple<ytp_peer_t, std::string_view>;
    using Stream = std::tuple<ytp_peer_t, ytp_channel_t, uint64_t,
                              std::string_view, std::string_view>;
    using Ch =
        std::tuple<ytp_peer_t, ytp_channel_t, uint64_t, std::string_view>;
    using Sub = std::tuple<uint64_t, ytp_peer_t, ytp_channel_t>;
    using D = std::tuple<ytp_peer_t, ytp_channel_t, uint64_t, std::string_view>;
    using V = std::variant<P, Stream, Sub, D>;
    std::vector<V> events;
    {
      auto cb =
          helper.peercb([&](ytp_peer_t peer, size_t sz, const char *name) {
            events.push_back(P(peer, {name, sz}));
          });
      ytp_timeline_peer_cb(timeline, cb.first, cb.second, &error);
      ASSERT_EQ(error, nullptr);
    }

    {
      auto cb = helper.streamcb([&](ytp_peer_t peer, ytp_channel_t channel,
                                    uint64_t msgtime, size_t chname_sz,
                                    const char *chname_ptr, size_t encoding_sz,
                                    const char *encoding) {
        auto chname = std::string_view(chname_ptr, chname_sz);
        events.push_back(
            Stream(peer, channel, msgtime, chname, {encoding, encoding_sz}));

        if (peer != YTP_PEER_OFF || channel != YTP_CHANNEL_OFF) {
          auto cb = helper.datacb(
              [&, chname](ytp_peer_t peer, ytp_channel_t channel,
                          uint64_t msgtime, size_t sz, const char *data) {
                size_t namesz;
                const char *name;

                ytp_control_ch_name(ctrl, channel, &namesz, &name, &error);
                ASSERT_EQ(error, nullptr);
                ASSERT_EQ(std::string_view(name, namesz), chname);

                events.push_back(D(peer, channel, msgtime, {data, sz}));
              });
          ytp_timeline_data_cb(timeline, 5100, peer, channel, cb.first,
                               cb.second, &error);
          ASSERT_EQ(error, nullptr);
        }
      });
      ytp_timeline_stream_cb(timeline, cb.first, cb.second, &error);
      ASSERT_EQ(error, nullptr);
    }

    {
      auto cb = helper.chcb([&](ytp_peer_t peer, ytp_channel_t channel,
                                uint64_t msgtime, size_t chname_sz,
                                const char *chname_ptr) {
        auto chname = std::string_view(chname_ptr, chname_sz);
        events.push_back(Ch(peer, channel, msgtime, chname));
      });
      ytp_timeline_ch_cb(timeline, cb.first, cb.second, &error);
      ASSERT_EQ(error, nullptr);
    }

    {
      auto cb = helper.subcb(
          [&](uint64_t msgtime, ytp_peer_t peer, ytp_channel_t channel) {
            events.push_back(Sub(msgtime, peer, channel));
          });
      ytp_timeline_sub_cb(timeline, cb.first, cb.second, &error);
      ASSERT_EQ(error, nullptr);
    }

    while (ytp_timeline_poll(timeline, &error))
      ;
    ASSERT_EQ(error, nullptr);

    ASSERT_EQ(events.size(), 17);
    auto p = events.begin();
    EXPECT_EQ(*(p++), V(P(256, "peer1")));
    EXPECT_EQ(*(p++), V(P(257, "peer2")));
    EXPECT_EQ(*(p++), V(Ch(256, 256, 5001, "ch1")));
    EXPECT_EQ(*(p++), V(Stream(256, 256, 5001, "ch1", "encoding1")));
    EXPECT_EQ(*(p++), V(Ch(256, 257, 5002, "ch2")));
    EXPECT_EQ(*(p++), V(Stream(256, 257, 5002, "ch2", "encoding2")));
    EXPECT_EQ(*(p++), V(Stream(257, 256, 5003, "ch1", "encoding3")));
    EXPECT_EQ(*(p++), V(D(257, 256, 5006, "0001")));
    EXPECT_EQ(*(p++), V(D(256, 257, 5007, "0002")));
    EXPECT_EQ(*(p++), V(Stream(257, 257, 5008, "ch2", "encoding4")));
    EXPECT_EQ(*(p++), V(D(257, 257, 5009, "0003")));
    EXPECT_EQ(*(p++), V(Sub(5011, 256, 256)));
    EXPECT_EQ(*(p++), V(Sub(5012, 257, 257)));
    EXPECT_EQ(*(p++), V(P(258, "peer3")));
    EXPECT_EQ(*(p++), V(Stream(258, 256, 5001, "ch1", "encoding1")));
    EXPECT_EQ(*(p++), V(D(256, 257, 5009, "0005")));
    EXPECT_EQ(*(p++), V(D(258, 256, 5009, "0006")));

    ytp_timeline_del(timeline, &error);
    ASSERT_EQ(error, nullptr);
  }

  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  {
    auto yamal = ytp_yamal_new(fd, &error);
    ASSERT_EQ(error, nullptr);

    ytp_peer_t peer;
    ytp_channel_t channel;
    uint64_t msgtime;
    size_t sz;
    const char *ptr;

    ytp_iterator_t it = ytp_yamal_begin(yamal, &error);
    ASSERT_EQ(error, nullptr);

    using T = std::tuple<ytp_peer_t, ytp_channel_t, uint64_t, std::string_view>;
    std::vector<T> messages;

    while (!ytp_yamal_term(it)) {
      ytp_time_read(yamal, it, &peer, &channel, &msgtime, &sz, &ptr, &error);
      ASSERT_EQ(error, nullptr);

      messages.push_back({peer, channel, msgtime, {ptr, sz}});

      it = ytp_yamal_next(yamal, it, &error);
      ASSERT_EQ(error, nullptr);
    }

    ASSERT_EQ(messages.size(), 21);
    auto p = messages.begin();
    EXPECT_EQ(*(p++), T(YTP_PEER_ANN, 0, 0, tostr("peer1")));
    EXPECT_EQ(*(p++), T(YTP_PEER_ANN, 0, 0, tostr("peer2")));
    EXPECT_EQ(*(p++),
              T(256, YTP_CHANNEL_ANN, 5001, tostr("\x3\0ch1encoding1")));
    EXPECT_EQ(*(p++),
              T(256, YTP_CHANNEL_ANN, 5002, tostr("\x3\0ch2encoding2")));
    EXPECT_EQ(*(p++),
              T(257, YTP_CHANNEL_ANN, 5003, tostr("\x3\0ch1encoding3")));
    EXPECT_EQ(*(p++), T(256, 256, 5005, tostr("0000")));
    EXPECT_EQ(*(p++), T(257, 256, 5006, tostr("0001")));
    EXPECT_EQ(*(p++), T(256, 257, 5007, tostr("0002")));
    EXPECT_EQ(*(p++),
              T(257, YTP_CHANNEL_ANN, 5008, tostr("\x3\0ch2encoding4")));
    EXPECT_EQ(*(p++), T(257, 257, 5009, tostr("0003")));
    EXPECT_EQ(*(p++),
              T(256, YTP_CHANNEL_SUB, 5011, tostr("\0\x1\0\0\0\0\0\0")));
    EXPECT_EQ(*(p++),
              T(257, YTP_CHANNEL_SUB, 5012, tostr("\x1\x1\0\0\0\0\0\0")));
    EXPECT_EQ(*(p++), T(YTP_PEER_ANN, 0, 0, "peer3"));
    EXPECT_EQ(*(p++),
              T(256, YTP_CHANNEL_ANN, 5001, tostr("\x3\0ch1encoding1")));
    EXPECT_EQ(*(p++),
              T(258, YTP_CHANNEL_ANN, 5001, tostr("\x3\0ch1encoding1")));
    EXPECT_EQ(*(p++), T(256, 256, 5009, tostr("0004")));
    EXPECT_EQ(*(p++), T(256, 257, 5009, tostr("0005")));
    EXPECT_EQ(*(p++), T(258, 256, 5009, tostr("0006")));
    EXPECT_EQ(*(p++),
              T(256, YTP_CHANNEL_SUB, 5100, tostr("\x1\x1\0\0\0\0\0\0")));
    EXPECT_EQ(*(p++),
              T(257, YTP_CHANNEL_SUB, 5100, tostr("\0\x1\0\0\0\0\0\0")));
    EXPECT_EQ(*(p++),
              T(258, YTP_CHANNEL_SUB, 5100, tostr("\0\x1\0\0\0\0\0\0")));

    ytp_yamal_del(yamal, &error);
    ASSERT_EQ(error, nullptr);
  }

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_rm_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ytp_timeline_prfx_cb_rm(timeline, 5, "main/", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_simple_subscription_rm_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ytp_timeline_indx_cb_rm(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel2, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_3) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_channel_4) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel2, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "EFGH");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_producers_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_control_commit(ctrl, producer2, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_multiple_producers_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_control_commit(ctrl, producer2, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_subscription_first_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb1 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb1, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {};

  ytp_timeline_prfx_cb(timeline, 5, "main/", cb2, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);
  auto channel3 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel3", &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);
  ASSERT_NE(channel3, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "IJKL");
  ytp_control_commit(ctrl, producer1, channel3, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output[0], "ABCD");
  ASSERT_EQ(output[1], "IJKL");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_subscription_first_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::string_view> output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (vector<std::string_view> *)closure;
    output->emplace_back(std::string_view(data, sz));
  };

  ytp_timeline_prfx_cb(timeline, 13, "main/channel1", cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 18, "secondary/channel2", &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_NE(channel2, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer1, channel2, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output[0], "ABCD");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, peer_simple) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };

  ytp_timeline_peer_cb(timeline, peer_cb, &output, &error);

  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(producer2, 0);

  const char *name;
  size_t sz;
  ytp_control_peer_name(ctrl, producer1, &sz, &name, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(std::string_view(name, sz), "producer1");
  ytp_control_peer_name(ctrl, 555, &sz, &name, &error);
  ASSERT_NE(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ytp_timeline_peer_cb_rm(timeline, peer_cb, &output, &error);

  (void)ytp_control_peer_decl(ctrl, 9, "producer3", &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(std::get<0>(output[0]), "producer1");
  ASSERT_EQ(std::get<1>(output[0]), producer1);

  ASSERT_EQ(std::get<0>(output[1]), "producer2");
  ASSERT_EQ(std::get<1>(output[1]), producer2);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
}

TEST(timeline, idempotence_simple_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  auto consumer1_2 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  auto producer1_2 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  auto channel1_2 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(consumer1, consumer1_2);
  ASSERT_EQ(producer1, producer1_2);
  ASSERT_EQ(channel1, channel1_2);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  strcpy(dst, "ABCD");
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  auto cb2 = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
                uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel1_2, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel1, cb2, &output, &error);
  ytp_timeline_indx_cb(timeline, channel1_2, cb2, &output, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  // consumer2
  (void)ytp_control_peer_decl(ctrl, 9, "consumer2", &error);
  // consumer2_2
  (void)ytp_control_peer_decl(ctrl, 9, "consumer2", &error);
  auto producer2 = ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  // producer2_2
  (void)ytp_control_peer_decl(ctrl, 9, "producer2", &error);
  auto channel2 =
      ytp_control_ch_decl(ctrl, consumer1, 1000, 13, "main/channel2", &error);
  auto channel2_2 =
      ytp_control_ch_decl(ctrl, consumer1, 1000, 13, "main/channel2", &error);

  ytp_timeline_indx_cb(timeline, channel2, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel2_2, cb, &output, &error);
  ytp_timeline_indx_cb(timeline, channel2, cb2, &output, &error);
  ytp_timeline_indx_cb(timeline, channel2_2, cb2, &output, &error);

  dst = ytp_control_reserve(ctrl, 4, &error);
  strcpy(dst, "EFGH");
  ytp_control_commit(ctrl, producer2, channel2, 1000, dst, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 4);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "ABCD");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "EFGH");
  ASSERT_EQ(std::get<1>(output[2]), producer2);
  ASSERT_EQ(std::get<2>(output[2]), channel2);
  ASSERT_EQ(std::get<3>(output[2]), 1000);

  ASSERT_EQ(std::get<0>(output[3]), "EFGH");
  ASSERT_EQ(std::get<1>(output[3]), producer2);
  ASSERT_EQ(std::get<2>(output[3]), channel2);
  ASSERT_EQ(std::get<3>(output[3]), 1000);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  ytp_peer_t peer;
  ytp_channel_t channel;
  uint64_t time;
  size_t sz;
  const char *data;
  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_NE(yamal, nullptr);

  auto *it = ytp_yamal_begin(yamal, &error);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, consumer1);
  ASSERT_EQ(channel, YTP_CHANNEL_ANN);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, producer1);
  ASSERT_EQ(channel, channel1);
  ASSERT_EQ(std::string_view(data, sz), "ABCD");
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, 0);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, consumer1);
  ASSERT_EQ(channel, YTP_CHANNEL_ANN);
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_FALSE(ytp_yamal_term(it));
  ytp_time_read(yamal, it, &peer, &channel, &time, &sz, &data, &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_EQ(peer, producer2);
  ASSERT_EQ(channel, channel2);
  ASSERT_EQ(std::string_view(data, sz), "EFGH");
  it = ytp_yamal_next(yamal, it, &error);
  ASSERT_NE(it, nullptr);

  ASSERT_TRUE(ytp_yamal_term(it));

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
}

TEST(timeline, idempotence_simple_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *yamal = ytp_yamal_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  ytp_peer_name(yamal, 5, "peer1", &error);
  ytp_peer_name(yamal, 5, "peer1", &error);

  ytp_yamal_del(yamal, &error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t>> output;

  auto peer_cb = [](void *closure, ytp_peer_t peer, size_t sz,
                    const char *name) {
    auto *output =
        (std::vector<std::tuple<std::string_view, ytp_peer_t>> *)closure;
    output->emplace_back(std::tuple<std::string_view, ytp_peer_t>(
        std::string_view(name, sz), peer));
  };

  ytp_timeline_peer_cb(timeline, peer_cb, &output, &error);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(std::get<0>(output[0]), "peer1");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, leading_slash_test) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 9, "/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  ytp_timeline_indx_cb(
      timeline, channel1,
      [](void *closure, ytp_peer_t peer, ytp_channel_t channel, uint64_t time,
         size_t sz, const char *data) {},
      nullptr, &error);
  ASSERT_EQ(error, nullptr);

  ytp_timeline_prfx_cb(
      timeline, 1, "a/",
      [](void *closure, ytp_peer_t peer, ytp_channel_t channel, uint64_t time,
         size_t sz, const char *data) {},
      nullptr, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  (void)ytp_control_ch_decl(ctrl, consumer1, 0, 9, "/channel2", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);

  while (ytp_timeline_poll(timeline, &error))
    ;
  ASSERT_EQ(error, nullptr);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, read_only_1) {
  fmc_error_t *error;

  fs::path filename = "readonly.ytp";

  fs::remove(filename);
  auto fd = fmc_fopen(filename.c_str(), fmc_fmode::READWRITE, &error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(error, nullptr);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  ytp_control_commit(ctrl, consumer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
  fd = fmc_fopen(filename.c_str(), fmc_fmode::READ, &error);

  ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(channel1, 0);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  (void)ytp_control_peer_decl(ctrl, 9, "consumer2", &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  (void)ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel2", &error);
  ASSERT_NE(error, nullptr);

  ASSERT_EQ(string_view(fmc_error_msg(error)).substr(0, 52),
            "unable to reserve using a readonly file descriptor (");

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
  fs::remove(filename);
}

TEST(timeline, data_iter_set_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto consumer1 = ytp_control_peer_decl(ctrl, 9, "consumer1", &error);
  ASSERT_EQ(error, nullptr);
  auto producer1 = ytp_control_peer_decl(ctrl, 9, "producer1", &error);
  ASSERT_EQ(error, nullptr);
  auto channel1 =
      ytp_control_ch_decl(ctrl, consumer1, 0, 13, "main/channel1", &error);
  ASSERT_EQ(error, nullptr);
  ASSERT_NE(consumer1, 0);
  ASSERT_NE(producer1, 0);
  ASSERT_NE(channel1, 0);

  char *dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "ABCD", 4);
  auto first_iter =
      ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "EFGH", 4);
  auto second_iter =
      ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  dst = ytp_control_reserve(ctrl, 4, &error);
  ASSERT_EQ(error, nullptr);
  memcpy(dst, "IJKL", 4);
  ytp_control_commit(ctrl, producer1, channel1, 1000, dst, &error);
  ASSERT_EQ(error, nullptr);

  std::vector<std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>>
      output;

  auto cb = [](void *closure, ytp_peer_t peer, ytp_channel_t channel,
               uint64_t time, size_t sz, const char *data) {
    auto *output = (std::vector<std::tuple<std::string_view, ytp_peer_t,
                                           ytp_channel_t, uint64_t>> *)closure;
    output->emplace_back(
        std::tuple<std::string_view, ytp_peer_t, ytp_channel_t, uint64_t>(
            std::string_view(data, sz), peer, channel, time));
  };

  ytp_timeline_indx_cb(timeline, channel1, cb, &output, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 3);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "EFGH");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "IJKL");
  ASSERT_EQ(std::get<1>(output[2]), producer1);
  ASSERT_EQ(std::get<2>(output[2]), channel1);
  ASSERT_EQ(std::get<3>(output[2]), 1000);
  output.clear();

  ytp_timeline_iter_set(timeline, first_iter);

  auto first_off =
      ytp_timeline_tell(timeline, ytp_timeline_iter_get(timeline), &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 3);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "EFGH");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "IJKL");
  ASSERT_EQ(std::get<1>(output[2]), producer1);
  ASSERT_EQ(std::get<2>(output[2]), channel1);
  ASSERT_EQ(std::get<3>(output[2]), 1000);
  output.clear();

  ytp_timeline_iter_set(timeline, second_iter);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(std::get<0>(output[0]), "EFGH");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "IJKL");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);
  output.clear();

  auto last_off =
      ytp_timeline_tell(timeline, ytp_timeline_iter_get(timeline), &error);
  ASSERT_EQ(error, nullptr);

  ytp_timeline_iter_set(timeline, second_iter);

  ytp_timeline_seek(timeline, last_off, &error);
  ASSERT_EQ(error, nullptr);

  ASSERT_EQ(output.size(), 0);

  ytp_timeline_seek(timeline, first_off, &error);
  ASSERT_EQ(error, nullptr);

  while (ytp_timeline_poll(timeline, &error))
    ;

  ASSERT_EQ(output.size(), 3);
  ASSERT_EQ(std::get<0>(output[0]), "ABCD");
  ASSERT_EQ(std::get<1>(output[0]), producer1);
  ASSERT_EQ(std::get<2>(output[0]), channel1);
  ASSERT_EQ(std::get<3>(output[0]), 1000);

  ASSERT_EQ(std::get<0>(output[1]), "EFGH");
  ASSERT_EQ(std::get<1>(output[1]), producer1);
  ASSERT_EQ(std::get<2>(output[1]), channel1);
  ASSERT_EQ(std::get<3>(output[1]), 1000);

  ASSERT_EQ(std::get<0>(output[2]), "IJKL");
  ASSERT_EQ(std::get<1>(output[2]), producer1);
  ASSERT_EQ(std::get<2>(output[2]), channel1);
  ASSERT_EQ(std::get<3>(output[2]), 1000);
  output.clear();

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_callback_removal_1) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto peer = ytp_control_peer_decl(ctrl, 4, "peer", &error);
  ASSERT_EQ(error, nullptr);
  auto ch = ytp_control_ch_decl(ctrl, peer, 0, 2, "ch", &error);
  ASSERT_EQ(error, nullptr);

  struct callback_t {
    static void c_callback(void *closure, ytp_peer_t peer,
                           ytp_channel_t channel, uint64_t time, size_t sz,
                           const char *data) {
      auto &self = *reinterpret_cast<callback_t *>(closure);
      self.callback();
    }
    std::function<void()> callback;
  };

  std::multiset<int> execution;
  std::multiset<int> expected;

  callback_t c1{[&]() { execution.insert(1); }};

  callback_t c2{[&]() { execution.insert(2); }};

  callback_t c4{[&]() { execution.insert(4); }};

  callback_t c3{[&]() {
    execution.insert(3);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c4, &error);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c1, &error);
  }};

  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c1, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c2, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c3, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c4, &error);

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {1, 2, 3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {2, 3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c2, &error);

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {3};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

TEST(timeline, data_callback_removal_2) {
  fmc_error_t *error;
  auto fd = fmc_ftemp(&error);
  ASSERT_EQ(error, nullptr);

  auto *ctrl = ytp_control_new(fd, &error);
  ASSERT_EQ(error, nullptr);

  auto *timeline = ytp_timeline_new(ctrl, &error);
  ASSERT_EQ(error, nullptr);

  auto peer = ytp_control_peer_decl(ctrl, 4, "peer", &error);
  ASSERT_EQ(error, nullptr);
  auto ch = ytp_control_ch_decl(ctrl, peer, 0, 2, "ch", &error);
  ASSERT_EQ(error, nullptr);

  struct callback_t {
    static void c_callback(void *closure, ytp_peer_t peer,
                           ytp_channel_t channel, uint64_t time, size_t sz,
                           const char *data) {
      auto &self = *reinterpret_cast<callback_t *>(closure);
      self.callback();
    }
    std::function<void()> callback;
  };

  std::vector<int> execution;
  std::vector<int> expected;

  callback_t c1{[&]() {
    execution.emplace_back(1);
    while (ytp_timeline_poll(timeline, &error))
      ;
  }};

  callback_t c3{[&]() { execution.emplace_back(3); }};

  callback_t c2{[&]() {
    execution.emplace_back(2);
    ytp_timeline_indx_cb_rm(timeline, ch, callback_t::c_callback, &c3, &error);
  }};

  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c1, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c2, &error);
  ytp_timeline_indx_cb(timeline, ch, callback_t::c_callback, &c3, &error);

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {1, 1, 2, 2};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_control_commit(ctrl, peer, ch, 1000, ytp_control_reserve(ctrl, 1, &error),
                     &error);
  ASSERT_EQ(error, nullptr);
  while (ytp_timeline_poll(timeline, &error))
    ;

  expected = {1, 2};
  EXPECT_EQ(execution, expected);
  execution.clear();

  ytp_timeline_del(timeline, &error);
  ASSERT_EQ(error, nullptr);
  ytp_control_del(ctrl, &error);
  ASSERT_EQ(error, nullptr);
  fmc_fclose(fd, &error);
  ASSERT_EQ(error, nullptr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
