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

#ifndef __FM_YTP_TIMELINE_HPP__
#define __FM_YTP_TIMELINE_HPP__

#include <apr.h> // apr_size_t APR_DECLARE
#include <ytp/channel.h>
#include <ytp/control.h>
#include <ytp/peer.h>
#include <ytp/timeline.h>
#include <ytp/yamal.h>

#include <stdbool.h> 
#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory> // std::unique_ptr
#include <utility> // std::pair

typedef std::pair<ytp_timeline_peer_cb_t, void *> ytp_timeline_peer_cb_cl_t;
typedef std::pair<ytp_timeline_ch_cb_t, void *> ytp_timeline_ch_cb_cl_t;
typedef std::pair<ytp_timeline_data_cb_t, void *> ytp_timeline_data_cb_cl_t;
typedef std::pair<ytp_timeline_idle_cb_t, void *> ytp_timeline_idle_cb_cl_t;

typedef ytp_channel_t ch_key;
typedef std::string prfx_cb_key;

template <typename T> struct as_ref : private T {
  using T::T;
  T *operator->() noexcept { return this; }
};

template <typename K, typename T>
struct stable_map : private std::unordered_map<K, std::unique_ptr<T>> {
  using std::unordered_map<K, std::unique_ptr<T>>::unordered_map;

  using M = std::unordered_map<K, std::unique_ptr<T>>;

  using base_iterator = typename M::iterator;
  using value_type = T;

  struct iterator : base_iterator {
    iterator(base_iterator &&base) : base_iterator(std::move(base)) {}

    std::pair<const K &, T &> operator*() noexcept {
      base_iterator &base = *this;
      return {base->first, *base->second};
    }
    as_ref<std::pair<const K &, T &>> operator->() noexcept {
      base_iterator &base = *this;
      return {base->first, *base->second};
    }
  };

  iterator find(const K &key) { return M::find(key); }

  T &operator[](const K &key) {
    auto where = M::emplace(key, std::unique_ptr<T>{});
    if (where.second) {
      where.first->second = std::make_unique<T>();
    }
    return *where.first->second;
  }

  iterator begin() noexcept { return M::begin(); }

  iterator end() noexcept { return M::end(); }
};

template <typename T>
struct lazy_rem_vector : private std::vector<std::pair<T, bool>> {
  using V = std::vector<std::pair<T, bool>>;
  using std::vector<std::pair<T, bool>>::vector;

  using base_iterator = typename V::iterator;

  struct iterator : base_iterator {
    iterator(base_iterator &&base) : base_iterator(std::move(base)) {}

    T &operator*() noexcept {
      base_iterator &base = *this;
      return base->first;
    }
    T *operator->() noexcept {
      base_iterator &base = *this;
      return &base->first;
    }

    bool was_removed() {
      base_iterator &base = *this;
      return base->second;
    }
  };

  lazy_rem_vector(const lazy_rem_vector<T> &) = delete;
  void operator=(const lazy_rem_vector<T> &) = delete;

  iterator begin() noexcept { return V::begin(); }

  iterator end() noexcept { return V::end(); }

  template <typename... Args> void emplace_back(Args &&... args) {
    V::emplace_back(T{std::forward<Args>(args)...}, false);
  }

  template <typename Arg> void push_unique(Arg &&new_val) {
    for (auto &val : static_cast<V &>(*this)) {
      if (new_val == val.first) {
        if (val.second) {
          --removed_count;
        }
        val.second = false;
        return;
      }
    }

    V::emplace_back(T{std::forward<Arg>(new_val)}, false);
  }

  template <typename F> void erase_if(const F &f) {
    if (lock_count == 0) {
      V::erase(std::remove_if(V::begin(), V::end(),
                              [&](const typename V::value_type &val) {
                                return f(val.first);
                              }),
               V::end());
    } else {
      for (auto &val : static_cast<V &>(*this)) {
        if (f(val.first)) {
          if (!val.second) {
            ++removed_count;
          }
          val.second = true;
        }
      }
    }
  }

  void clear() {
    if (lock_count == 0) {
      V::clear();
    } else {
      for (auto &val : static_cast<V &>(*this)) {
        if (!val.second) {
          ++removed_count;
        }
        val.second = true;
      }
    }
  }

  void lock() { ++lock_count; }

  void release() {
    if (--lock_count == 0) {
      if (removed_count > 0) {
        removed_count = 0;

        V::erase(std::remove_if(V::begin(), V::end(),
                                [&](const typename V::value_type &val) {
                                  return val.second;
                                }),
                 V::end());
      }
    }
  }

  apr_size_t lock_count = 0;
  apr_size_t removed_count = 0;
};

struct ytp_timeline {
  ytp_control *ctrl;
  ytp_iterator_t read;

  lazy_rem_vector<ytp_timeline_peer_cb_cl_t> cb_peer;
  lazy_rem_vector<ytp_timeline_ch_cb_cl_t> cb_ch;
  std::unordered_map<prfx_cb_key, lazy_rem_vector<ytp_timeline_data_cb_cl_t>>
      prfx_cb;
  stable_map<ch_key, lazy_rem_vector<ytp_timeline_data_cb_cl_t>> idx_cb;
  lazy_rem_vector<ytp_timeline_idle_cb_cl_t> cb_idle;
  std::vector<uint8_t> ch_announced;
  std::vector<uint8_t> peer_announced;
  std::unordered_set<std::string_view> sub_announced;
};

#endif // __FM_YTP_TIMELINE_HPP__
