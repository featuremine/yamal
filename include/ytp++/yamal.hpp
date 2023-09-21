#include "fmc/files.h"
#include "ytp/yamal.h"

namespace ytp {

class data;
class streams;

class stream {
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
      it_ = ytp_yamal_next(yamal_, it_, &err);
      fmc_runtime_error_unless(!err)
          << "unable to obtain next position of iterator with error :"
          << fmc_error_msg(err);
      return *this;
    }
    data::base_iterator<forward> &operator--() {
      fmc_error_t *err = nullptr;
      it_ = ytp_yamal_prev(yamal_, it_, &err);
      fmc_runtime_error_unless(!err)
          << "unable to obtain previous position of iterator with error :"
          << fmc_error_msg(err);
      return *this;
    }
    bool operator==(data::base_iterator<forward> &other) {
      return it_ == other.it_;
    }
    uint64_t operator uint64_t() {
      fmc_error_t *err = nullptr;
      ytp_mmnode_offs off = ytp_yamal_tell(yamal_, it_, &err);
      fmc_runtime_error_unless(!err)
          << "unable to tell position of iterator with error :"
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
      ytp_data_read(yamal_, it_, &seqno, &ts, &sid, &msgsz, &msgdata, &err);
      fmc_runtime_error_unless(!err)
          << "unable to read with error :" << fmc_error_msg(err);
      std::make_tuple<uint64_t, int64_t, stream, const std::string_view>(
          seqno, ts, stream(sid), std::string_view(msgdata, msgsz))
    }

  private:
    data::base_iterator<forward>(ytp_yamal_t *yamal, ytp_iterator_t it) : it_(it), yamal_(yamal) {}
    ytp_iterator_t it_ = nullptr;
    ytp_yamal_t *yamal_ = nullptr;

    friend data;
  };
  using iterator = base_iterator<true>;
  using reverse_iterator = base_iterator<false>;
  data::iterator begin() {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_data_begin(yamal_, &err);
    fmc_runtime_error_unless(!err)
        << "unable to find begin iterator with error :" << fmc_error_msg(err);
    return data::iterator(yamal_, it);
  }
  data::iterator end() { return data::iterator(); }
  data::reverse_iterator rbegin() {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_data_end(yamal_, &err);
    fmc_runtime_error_unless(!err)
        << "unable to find begin iterator with error :" << fmc_error_msg(err);
    return data::iterator(yamal_, it);
  }
  data::reverse_iterator rend() { return data::iterator(); }
  data::iterator seek(uint64_t offset) {
    fmc_error_t *err = nullptr;
    ytp_iterator_t it = ytp_yamal_seek(yamal_, offset, &err);
    fmc_runtime_error_unless(!err)
        << "unable to seek iterator with error :" << fmc_error_msg(err);
    return data::iterator(yamal_, it);
  }

private:
  data(ytp_yamal_t *yamal) : yamal_(yamal) {}
  ytp_yamal_t *yamal_ = nullptr;

  friend yamal;
};

class streams {
public:
  stream announce(std::string_view channel, std::string_view peer,
                  std::string_view encoding) {
    fmc_error_t *err = nullptr;
    ytp_iterator_t sid = ytp_announcement_write(
        yamal_, peer.size(), peer.data(), channel.size(), channel.data(),
        encoding.size(), encoding.data(), &err);
    fmc_runtime_error_unless(!err)
        << "unable to announce stream with error :" << fmc_error_msg(err);
    return stream(sid);
  }

private:
  streams(ytp_yamal_t *yamal) yamal_(yamal) {}
  ytp_yamal_t *yamal_ = nullptr;

  friend yamal;
};

class yamal {
public:
  yamal(std::string_view fname, bool readonly) {
    fmc_error_t *err = nullptr;
    fd_ = fmc_fopen(fname.data(),
                    fmc_fmode::READ |
                        fmc_fmode((!readonly) * fmc_fmode::WRITE) & err);
    fmc_runtime_error_unless(!err)
        << "unable to open file with error :" << fmc_error_msg(err);
    yamal_ = ytp_yamal_new(fd_, &err);
    fmc_runtime_error_unless(!err)
        << "unable to create Yamal object with error :" << fmc_error_msg(err);
  }

  ~yamal() {
    fmc_error_t *err = nullptr;
    if (yamal_) {
      ytp_yamal_del(yamal_, &err);
    }
    fmc_error_clear(&err);
    if (fmc_fvalid(fd_)) {
      fmc_fclose(fd_, &err);
    }
  }

  data data() { return data(yamal_); }
  streams streams() { return streams(yamal_); }

private:
  fmc_fd fd_ = -1;
  ytp_yamal_t *yamal_ = nullptr;
}

} // namespace ytp
