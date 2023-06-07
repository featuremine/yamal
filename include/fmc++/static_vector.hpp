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
 * @file include/fmc++/static_vector.hpp
 * @date Oct 25 2022
 */

#pragma once

#include <fmc++/mpl.hpp>

#include <algorithm>
#include <initializer_list>
#include <iterator>

namespace fmc {

template <typename T, std::size_t N> struct static_vector {
  typedef T value_type;
  typedef value_type *pointer;
  typedef const value_type *const_pointer;
  typedef value_type &reference;
  typedef const value_type &const_reference;
  typedef value_type *iterator;
  typedef const value_type *const_iterator;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  static_vector() noexcept : end_iterator(begin()) {}
  ~static_vector() noexcept { clear(); }
  static_vector(std::size_t n) noexcept : end_iterator(begin() + n) {
    for (size_type i = size_type(); i < size(); ++i) {
      new (&payload.arr[i]) T();
    }
  }
  static_vector(std::initializer_list<value_type> l) : end_iterator(begin()) {
    for (auto &v : l) {
      push_back(v);
    }
  }
  static_vector(const static_vector &v) : end_iterator(begin() + v.size()) {
    for (size_type i = size_type(); i < size(); ++i) {
      new (&payload.arr[i]) T(v.payload.arr[i]);
    }
  }
  static_vector(static_vector &&v) noexcept : end_iterator(begin() + v.size()) {
    for (size_type i = size_type(); i < size(); ++i) {
      new (&payload.arr[i]) T(std::move(v.payload.arr[i]));
    }
    v.clear();
  }
  const static_vector &operator=(const static_vector &v) {
    auto assign_count = std::min(size(), v.size());
    for (size_type i = size_type(); i < assign_count; ++i) {
      payload.arr[i] = v.payload.arr[i];
    }
    for (size_type i = size(); i-- > assign_count;) {
      pop_back();
    }
    for (auto it = v.begin() + assign_count; it != v.end(); ++it) {
      push_back(*it);
    }
    return *this;
  }
  const static_vector &operator=(static_vector &&v) {
    auto assign_count = std::min(size(), v.size());
    for (size_type i = size_type(); i < assign_count; ++i) {
      payload.arr[i] = std::move(v.payload.arr[i]);
    }
    for (size_type i = size(); i-- > assign_count;) {
      pop_back();
    }
    for (auto it = v.begin() + assign_count; it != v.end(); ++it) {
      push_back(std::move(*it));
    }
    v.clear();
    return *this;
  }
  static_vector &operator=(std::initializer_list<value_type> l) {
    auto assign_count = std::min(size(), l.size());
    auto it = l.begin();
    for (size_type i = size_type(); i < assign_count; ++i) {
      payload.arr[i] = *(it++);
    }
    for (size_type i = size(); i-- > assign_count;) {
      pop_back();
    }
    for (; it != l.end(); ++it) {
      push_back(*it);
    }
    return *this;
  }

  constexpr void fill(const value_type &v) { std::fill(begin(), end(), v); }

  constexpr void clear() {
    for (size_type i = size(); i-- > size_type();) {
      pop_back();
    }
  }

  constexpr iterator begin() noexcept { return iterator(&payload.arr[0]); }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(&payload.arr[0]);
  }

  constexpr iterator end() noexcept { return end_iterator; }

  constexpr const_iterator end() const noexcept { return end_iterator; }

  constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  constexpr const_iterator cbegin() const noexcept {
    return const_iterator(data());
  }

  constexpr const_iterator cend() const noexcept {
    return const_iterator(end());
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] size_type size() const noexcept { return end() - begin(); }

  [[nodiscard]] constexpr size_type max_size() const noexcept { return N; }

  [[nodiscard]] bool empty() const noexcept { return size() == 0; }

  constexpr reference operator[](size_type n) noexcept {
    return payload.arr[n];
  }

  constexpr const_reference operator[](size_type n) const noexcept {
    return payload.arr[n];
  }

  constexpr reference at(size_type n) { return payload.arr[n]; }

  constexpr const_reference at(size_type n) const { return payload.arr[n]; }

  constexpr reference front() noexcept { return *begin(); }

  constexpr const_reference front() const noexcept { return *begin(); }

  constexpr reference back() noexcept { return *rend(); }

  constexpr const_reference back() const noexcept { return *rend(); }

  constexpr pointer data() noexcept { return &*begin(); }

  constexpr const_pointer data() const noexcept { return &*begin(); }

  void push_back(const value_type &v) { new (end_iterator++) T(v); }

  void push_back(value_type &&v) { new (end_iterator++) T(std::move(v)); }

  template <typename... Arg> void emplace_back(Arg &&...arg) {
    new (end_iterator++) T(std::forward<Arg>(arg)...);
  }

  void pop_back() { (--end_iterator)->~T(); }

private:
  struct empty_t {};
  union storage_t {
    constexpr storage_t() noexcept : empty() {}
    ~storage_t() {}
    empty_t empty;
    T arr[N];
  };
  storage_t payload;
  iterator end_iterator;
};

template <typename T, std::size_t N>
inline bool operator==(const static_vector<T, N> &lhs,
                       const static_vector<T, N> &rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
}

} // namespace fmc
