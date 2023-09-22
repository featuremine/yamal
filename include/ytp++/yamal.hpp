/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file yamal.hpp
 * @author Andres Rangel
 * @date 22 Sep 2022
 * @brief File contains C++ definitions for YTP yamal layer
 *
 * @see http://www.featuremine.com
 */

#include "fmc++/memory.hpp"
#include "fmc++/mpl.hpp"
#include "fmc/files.h"
#include "ytp/data.h"
#include "ytp/streams.h"
#include "ytp/yamal.h"

#include <string_view>
#include <utility>

namespace ytp {

class yamal;

class data;

class streams;

class stream {
public:
  ytp_mmnode_offs id() const { return id_; }
  stream(ytp_mmnode_offs id) : id_(id) {}
  stream(const stream &s) = default;
  stream(stream &&s) = default;
  bool operator==(const stream other) const { return id_ == other.id_; }

private:
  stream() = default;
  ytp_mmnode_offs id_;
};

class data {
public:
  template <bool forward> class base_iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::tuple<uint64_t, int64_t, stream, std::string_view>;
    using pointer = value_type *;
    using reference = value_type &;

    base_iterator() : it_(nullptr), yamal_(nullptr) {}
    base_iterator<forward> &operator++() {
      fmc_error_t *err = nullptr;
      if constexpr (forward) {
        it_ = ytp_yamal_next(yamal_.get(), it_, &err);
      } else {
        it_ = ytp_yamal_prev(yamal_.get(), it_, &err);
      }
      fmc_runtime_error_unless(!err)
          << "unable to obtain next position of iterator with error:"
          << fmc_error_msg(err);
      return *this;
    }
    base_iterator<forward> &operator--() {
      fmc_error_t *err = nullptr;
      if constexpr (forward) {
        it_ = ytp_yamal_prev(yamal_.get(), it_, &err);
      } else {
        it_ = ytp_yamal_next(yamal_.get(), it_, &err);
      }
      fmc_runtime_error_unless(!err)
          << "unable to obtain previous position of iterator with error:"
          << fmc_error_msg(err);
      return *this;
    }
    bool operator==(base_iterator<forward> &other) {
      if constexpr (forward) {
        if (it_ == nullptr) {
          return ytp_yamal_term(other.it_);
        } else if (other.it_ == nullptr) {
          return ytp_yamal_term(it_);
        }
      } else {
        fmc_error_t *err = nullptr;
        if (it_ == nullptr) {
          ytp_iterator_t first = ytp_data_begin(yamal_.get(), &err);
          fmc_runtime_error_unless(!err)
              << "unable to obtain begining of data with error:"
              << fmc_error_msg(err);
          ytp_iterator_t head = ytp_yamal_prev(yamal_.get(), first, &err);
          fmc_runtime_error_unless(!err)
              << "unable to obtain head of data with error:"
              << fmc_error_msg(err);
          return other.it_ == head;
        } else if (other.it_ == nullptr) {
          ytp_iterator_t first = ytp_data_begin(yamal_.get(), &err);
          fmc_runtime_error_unless(!err)
              << "unable to obtain begining of data with error:"
              << fmc_error_msg(err);
          ytp_iterator_t head = ytp_yamal_prev(yamal_.get(), first, &err);
          fmc_runtime_error_unless(!err)
              << "unable to obtain head of data with error:"
              << fmc_error_msg(err);
          return it_ == head;
        }
      }
      return it_ == other.it_;
    }
    operator ytp_mmnode_offs() {
      fmc_error_t *err = nullptr;
      ytp_mmnode_offs off = ytp_yamal_tell(yamal_.get(), it_, &err);
      fmc_runtime_error_unless(!err)
          << "unable to tell position of iterator with error:"
          << fmc_error_msg(err);
      return off;
    }
    value_type operator*() {
      fmc_error_t *err = nullptr;
      uint64_t seqno;
      int64_t ts;
      ytp_mmnode_offs sid;
      size_t msgsz;
      const char *msgdata;
      ytp_data_read(yamal_.get(), it_, &seqno, &ts, &sid, &msgsz, &msgdata,
                    &err);
      fmc_runtime_error_unless(!err)
          << "unable to read with error:" << fmc_error_msg(err);
      return value_type(seqno, ts, stream(sid),
                        std::string_view(msgdata, msgsz));
    }

  private:
    base_iterator(std::shared_ptr<ytp_yamal_t> yamal, ytp_iterator_t it)
        : it_(it), yamal_(yamal) {}
    ytp_iterator_t it_;
    std::shared_ptr<ytp_yamal_t> yamal_;

    friend data;
  };
  using iterator = base_iterator<true>;
  using reverse_iterator = base_iterator<false>;
  iterator begin() {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_data_begin(yamal_.get(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to find begin iterator with error:" << fmc_error_msg(err);
    return iterator(yamal_, it);
  }
  iterator end() { return iterator(); }
  reverse_iterator rbegin() {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_data_end(yamal_.get(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to find begin iterator with error:" << fmc_error_msg(err);
    // find previous to it and return THAT, we are after
    return reverse_iterator(yamal_, it);
  }
  reverse_iterator rend() { return reverse_iterator(); }
  iterator seek(ytp_mmnode_offs offset) {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_yamal_seek(yamal_.get(), offset, &err);
    fmc_runtime_error_unless(!err)
        << "unable to seek iterator with error:" << fmc_error_msg(err);
    return iterator(yamal_, it);
  }

  void close() {
    fmc_error_t *err = nullptr;
    ytp_yamal_close(yamal_.get(), YTP_STREAM_LIST_DATA, &err);
    fmc_runtime_error_unless(!err)
        << "unable to close yamal with error:" << fmc_error_msg(err);
  }
  bool closed() {
    fmc_error_t *err = nullptr;
    bool ret = ytp_yamal_closed(yamal_.get(), YTP_STREAM_LIST_DATA, &err);
    fmc_runtime_error_unless(!err)
        << "unable to validate if yamal is closed with error:"
        << fmc_error_msg(err);
    return ret;
  }
  bool closable() {
    fmc_error_t *err = nullptr;
    bool ret = ytp_yamal_closable(yamal_.get(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to validate if yamal is closable with error:"
        << fmc_error_msg(err);
    return ret;
  }

  fmc::buffer reserve(size_t sz) {
    fmc_error_t *err = nullptr;
    char *out = ytp_data_reserve(yamal_.get(), sz, &err);
    fmc_runtime_error_unless(!err)
        << "unable to reserve data with error:" << fmc_error_msg(err);
    return fmc::buffer(out, sz);
  }

  ytp_iterator_t commit(int64_t ts, stream s, fmc::buffer data) {
    fmc_error_t *err = nullptr;
    ytp_iterator_t ret =
        ytp_data_commit(yamal_.get(), ts, s.id(), data.data(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to commit data with error:" << fmc_error_msg(err);
    return ret;
  }

private:
  data(std::shared_ptr<ytp_yamal_t> yamal) : yamal_(yamal) {}
  std::shared_ptr<ytp_yamal_t> yamal_;

  friend yamal;
};

class streams {
public:
  stream announce(std::string_view peer, std::string_view channel,
                  std::string_view encoding) {
    fmc_error_t *err = nullptr;
    ytp_mmnode_offs sid = ytp_streams_announce(
        streams_.get(), peer.size(), peer.data(), channel.size(),
        channel.data(), encoding.size(), encoding.data(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to announce stream with error:" << fmc_error_msg(err);
    return stream(sid);
  }

  std::pair<stream, std::string_view> lookup(std::string_view peer,
                                             std::string_view channel) {
    fmc_error_t *err = nullptr;
    size_t esz = 0;
    const char *edata = nullptr;
    ytp_mmnode_offs sid =
        ytp_streams_lookup(streams_.get(), peer.size(), peer.data(),
                           channel.size(), channel.data(), &esz, &edata, &err);
    fmc_runtime_error_unless(!err)
        << "unable to look up stream with error:" << fmc_error_msg(err);
    return std::pair<stream, std::string_view>(stream(sid),
                                               std::string_view(edata, esz));
  }

private:
  streams(std::shared_ptr<ytp_yamal_t> yamal) : yamal_(yamal) {
    fmc_error_t *err = nullptr;
    streams_ = std::shared_ptr<ytp_streams_t>(
        ytp_streams_new(yamal_.get(), &err), [](auto ss) {
          fmc_error_t *err = nullptr;
          if (ss) {
            ytp_streams_del(ss, &err);
          }
        });
    fmc_runtime_error_unless(!err)
        << "unable to create streams object with error:" << fmc_error_msg(err);
  }
  std::shared_ptr<ytp_yamal_t> yamal_;
  std::shared_ptr<ytp_streams_t> streams_;

  friend yamal;
};

class yamal {
public:
  yamal(fmc_fd fd, bool closable = false) {
    fmc_error_t *err = nullptr;
    yamal_ =
        std::shared_ptr<ytp_yamal_t>(ytp_yamal_new(fd, &err), [](auto yml) {
          fmc_error_t *err = nullptr;
          if (yml) {
            ytp_yamal_del(yml, &err);
          }
        });
    fmc_runtime_error_unless(!err)
        << "unable to create Yamal object with error:" << fmc_error_msg(err);
  }

  ~yamal() = default;

  data data() { return ytp::data(yamal_); }
  streams streams() { return ytp::streams(yamal_); }

  // seqnum, peer, channel, encoding
  std::tuple<int, std::string_view, std::string_view, std::string_view>
  announcement(stream s) {
    fmc_error_t *err = nullptr;
    uint64_t seqno;
    size_t psz;
    const char *peer;
    size_t csz;
    const char *channel;
    size_t esz;
    const char *encoding;
    ytp_mmnode_offs *original;
    ytp_mmnode_offs *subscribed;
    ytp_announcement_lookup(yamal_.get(), s.id(), &seqno, &psz, &peer, &csz,
                            &channel, &esz, &encoding, &original, &subscribed,
                            &err);
    fmc_runtime_error_unless(!err)
        << "unable to create Yamal object with error:" << fmc_error_msg(err);
    return std::make_tuple<int, std::string_view, std::string_view,
                           std::string_view>(seqno, std::string_view(peer, psz),
                                             std::string_view(channel, csz),
                                             std::string_view(encoding, esz));
  }

  ytp_yamal_t *get() { return yamal_.get(); }

private:
  std::shared_ptr<ytp_yamal_t> yamal_;
};

} // namespace ytp

namespace std {

inline ostream &operator<<(ostream &os, const ytp::stream &s) {
  return os << s.id();
}

template <> struct hash<ytp::stream> {
  size_t operator()(const ytp::stream &s) const {
    return std::hash<ytp_mmnode_offs>{}(s.id());
  }
};

} // namespace std