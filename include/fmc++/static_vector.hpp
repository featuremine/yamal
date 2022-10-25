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

#include <array>

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
  static_vector(std::size_t n) noexcept : end_iterator(begin() + n) {
    fmc_runtime_error_unless(n <= N) << "static_vector overflow";
  }
  static_vector(const static_vector &v)
      : impl(v.impl), end_iterator(begin() + v.size()) {}
  static_vector(static_vector &&v) noexcept
      : impl(std::move(v.impl)), end_iterator(begin() + v.size()) {
    v.end_iterator = v.begin();
  }
  const static_vector &operator=(const static_vector &v) {
    impl = v.impl;
    end_iterator = begin() + v.size();
    return *this;
  }
  const static_vector &operator=(static_vector &&v) {
    impl = std::move(v.impl);
    end_iterator = begin() + v.size();
    v.end_iterator = v.begin();
    return *this;
  }

  constexpr void fill(const value_type &v) { std::fill_n(begin(), size(), v); }

  constexpr void clear() { end_iterator = begin(); }

  constexpr iterator begin() noexcept { return iterator(&impl[0]); }

  constexpr const_iterator begin() const noexcept {
    return const_iterator(&impl[0]);
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

  size_type size() const noexcept { return end() - begin(); }

  constexpr size_type max_size() const noexcept { return N; }

  bool empty() const noexcept { return size() == 0; }

  constexpr reference operator[](size_type n) noexcept {
    fmc_runtime_error_unless(n <= N) << "static_vector out of bounds";
    return impl[n];
  }

  constexpr const_reference operator[](size_type n) const noexcept {
    fmc_runtime_error_unless(n <= N) << "static_vector out of bounds";
    return impl[n];
  }

  constexpr reference at(size_type n) { return impl.at(n); }

  constexpr const_reference at(size_type n) const { return impl.at(n); }

  constexpr reference front() noexcept { return *begin(); }

  constexpr const_reference front() const noexcept { return *begin(); }

  constexpr reference back() noexcept { return *end(); }

  constexpr const_reference back() const noexcept { return *end(); }

  constexpr pointer data() noexcept { return &*begin(); }

  constexpr const_pointer data() const noexcept { return &*begin(); }

  void push_back(const value_type &v) {
    fmc_runtime_error_unless(size() < max_size()) << "static_vector overflow";
    *(end_iterator++) = v;
  }

  void push_back(value_type &&v) {
    fmc_runtime_error_unless(size() < max_size()) << "static_vector overflow";
    *(end_iterator++) = std::move(v);
  }

  void pop_back() { *(--end_iterator) = T(); }

private:
  std::array<T, N> impl;
  typename std::array<T, N>::iterator end_iterator;
};

template <typename T, std::size_t N>
inline bool operator==(const static_vector<T, N> &lhs,
                       const static_vector<T, N> &rhs) {
  return lhs.size() == rhs.size() && lhs.impl == rhs.impl;
}

} // namespace fmc
