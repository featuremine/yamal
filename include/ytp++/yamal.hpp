#include "fmc/files.h"
#include "ytp/yamal.h"

namespace ytp {

class data;
class streams;

class stream {
  // TODO: make hashable and serializable
  ytp_mmnode_offs id() {return id_;}
private:
  stream() = default;
  ytp_mmnode_offs id_;

  friend data;
  friend streams;
};

class yamal;

class data {
public:
  template <bool forward> class base_iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type =
        std::tuple<uint64_t, int64_t, stream, const std::string_view>;
    using pointer = value_type *;
    using reference = value_type &;

    data::base_iterator<forward>() : it_(nullptr), yamal_(nullptr) {}
    data::base_iterator<forward> &operator++() {
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
    data::base_iterator<forward> &operator--() {
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
    bool operator==(data::base_iterator<forward> &other) {
      // Todo: check for the end, either one could be null
      // Ensure we deal with forward:
      // - if forward iterator, check for term
      // - if reverse iterator, end would be at the head
      return it_ == other.it_;
    }
    ytp_mmnode_offs operator ytp_mmnode_offs() {
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
      ytp_data_read(yamal_.get(), it_, &seqno, &ts, &sid, &msgsz, &msgdata, &err);
      fmc_runtime_error_unless(!err)
          << "unable to read with error:" << fmc_error_msg(err);
      return std::make_tuple<uint64_t, int64_t, stream, const std::string_view>(
          seqno, ts, stream(sid), std::string_view(msgdata, msgsz))
    }

  private:
    data::base_iterator<forward>(std::shared_ptr<ytp_yamal_t> yamal, ytp_iterator_t it)
        : it_(it), yamal_(yamal) {}
    ytp_iterator_t it_ = nullptr;
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
    return iterator(yamal_.get(), it);
  }
  iterator end() { return iterator(); }
  reverse_iterator rbegin() {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_data_end(yamal_.get(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to find begin iterator with error:" << fmc_error_msg(err);
    // find previous to it and return THAT, we are after
    return reverse_iterator(yamal_.get(), it);
  }
  reverse_iterator rend() { return reverse_iterator(); }
  iterator seek(ytp_mmnode_offs offset) {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_yamal_seek(yamal_.get(), offset, &err);
    fmc_runtime_error_unless(!err)
        << "unable to seek iterator with error:" << fmc_error_msg(err);
    return iterator(yamal_.get(), it);
  }

  void close() {
    fmc_error_t *err = nullptr;
    ytp_yamal_close(ytp_yamal_t *yamal, YTP_STREAM_LIST_DATA, &err);
    fmc_runtime_error_unless(!err)
        << "unable to close yamal with error:" << fmc_error_msg(err);
    return ret;
  }
  bool closed() {
    fmc_error_t *err = nullptr;
    bool ret = ytp_yamal_closed(ytp_yamal_t *yamal, YTP_STREAM_LIST_DATA, &err);
    fmc_runtime_error_unless(!err)
        << "unable to validate if yamal is closed with error:" << fmc_error_msg(err);
    return ret;
  }
  bool closable() {
    fmc_error_t *err = nullptr;
    bool ret = ytp_yamal_closable(ytp_yamal_t *yamal, fmc_error_t **error);
    fmc_runtime_error_unless(!err)
        << "unable to validate if yamal is closable with error:" << fmc_error_msg(err);
    return ret;
  }

  void write(stream s, std::string_view data) {
    fmc_error_t *err = nullptr;
    char *out = ytp_data_reserve(yamal_.get(), data.sz(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to reserve data with error:" << fmc_error_msg(err);
    memcpy(out, data.data(), data.sz());
    ytp_iterator_t ytp_data_commit(yamal_.get(), fmc_cur_time_ns(),
                                   s.id(), out, &err);
    fmc_runtime_error_unless(!err)
        << "unable to commit data with error:" << fmc_error_msg(err);
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
    // Change streams functions
    ytp_iterator_t sid = ytp_announcement_write(
        yamal_.get(), peer.size(), peer.data(), channel.size(), channel.data(),
        encoding.size(), encoding.data(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to announce stream with error:" << fmc_error_msg(err);
    return stream(sid);
  }

  stream lookup(std::string_view peer, std::string_view channel,
                         std::string_view encoding) {
    ytp_mmnode_offs sid = ytp_streams_lookup(
        streams_.get(), peer.size(), peer.data(), channel.size(),
        channel.data(), encoding.size(), encoding.data(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to look up stream with error:" << fmc_error_msg(err);
    return stream(sid);
  }

private:
  streams(std::shared_ptr<ytp_yamal_t> yamal) yamal_(yamal) {
    fmc_error_t *err = nullptr;
    streams_ = ytp_streams_new(yamal_.get(), &err);
  }
  std::shared_ptr<ytp_yamal_t> yamal_;
  std::shared_ptr<ytp_streams_t> streams_;

  friend yamal;
};

class yamal {
public:
  yamal(fmc_fd fd, bool closable = false) {
    fmc_error_t *err = nullptr;
    yamal_ = std::shared_ptr<ytp_yamal_t>(ytp_yamal_new(fd, &err), [](auto yamal){
      fmc_error_t *err = nullptr;
      if (yamal) {
        ytp_yamal_del(yamal, &err);
      }
    });
    fmc_runtime_error_unless(!err)
        << "unable to create Yamal object with error:" << fmc_error_msg(err);
  }

  ~yamal() = default;

  data data() { return data(yamal_); }
  streams streams() { return streams(yamal_); }

  // seqnum, peer, channel, encoding
  tuple<int, std::string_view, std::string_view, std::string_view>
  announcement(stream s);

private:
  fmc_fd fd_ = -1;
  std::shared_ptr<ytp_yamal_t> yamal_;
  // use shared ptr, destructor will have deletion function that clears the
  // descriptor
}

} // namespace ytp
