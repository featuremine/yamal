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

#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace fmc {
template <typename T> struct lazy_rem_vector;
}

namespace std {
template <typename T, typename F>
void erase_if(fmc::lazy_rem_vector<T> &v, const F &f);
}

namespace fmc {

template <typename T>
struct lazy_rem_vector : private std::vector<std::pair<T, bool>> {
  using V = std::vector<std::pair<T, bool>>;
  using std::vector<std::pair<T, bool>>::vector;

  using base_iterator = typename V::iterator;

  struct iterator : base_iterator {
    iterator() = default;
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

  template <typename... Args> void emplace_back(Args &&...args) {
    V::emplace_back(T{std::forward<Args>(args)...}, false);
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

  bool empty() noexcept { return V::empty(); }

  typename V::size_type size() noexcept { return V::size(); }

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

protected:
  size_t lock_count = 0;
  size_t removed_count = 0;

  template <typename S, typename F>
  friend void ::std::erase_if(lazy_rem_vector<S> &v, const F &f);

  template <typename S, typename Arg>
  friend void push_unique(lazy_rem_vector<S> &, Arg &&);
};

template <typename T, typename Arg>
void push_unique(lazy_rem_vector<T> &l, Arg &&new_val) {
  using V = std::vector<std::pair<T, bool>>;
  V &base = l;
  for (auto &val : static_cast<V &>(l)) {
    if (new_val == val.first) {
      if (val.second) {
        --l.removed_count;
      }
      val.second = false;
      return;
    }
  }

  base.emplace_back(T{std::forward<Arg>(new_val)}, false);
}

template <typename T, typename Arg>
void push_unique(std::vector<T> &v, Arg &&new_val) {
  if (std::find(v.begin(), v.end(), new_val) != v.end()) {
    return;
  }
  v.emplace_back(std::forward<Arg>(new_val));
}

} // namespace fmc

namespace std {
template <typename T, typename F>
void erase_if(fmc::lazy_rem_vector<T> &v, const F &f) {
  using V = typename fmc::lazy_rem_vector<T>::V;
  V &base = v;
  if (v.lock_count == 0) {
    base.erase(remove_if(base.begin(), base.end(),
                         [&](const typename V::value_type &val) {
                           return f(val.first);
                         }),
               base.end());
  } else {
    for (auto &val : base) {
      if (f(val.first)) {
        if (!val.second) {
          ++v.removed_count;
        }
        val.second = true;
      }
    }
  }
}
} // namespace std
