/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file misc.hpp
 * @author Maxim Trokhimtchouk
 * @date 4 Oct 2017
 * @brief File contains C++ implementation of misc
 *
 * This file describes generic interface subscription mechanism
 */

#pragma once

#include <fmc++/mpl.hpp>
#include <fmc++/serialize.hpp>
#include <fmc/alignment.h>

#include <functional>
#include <optional>
#include <tuple>

#include <iostream>

#define NOCOPY(CL)                                                             \
  CL(const CL &) = delete;                                                     \
  CL &operator=(const CL &) = delete

namespace fmc {
using namespace std;

template <class T> class functional_iterator {
public:
  using iterator_category = input_iterator_tag;
  using value_type = T;
  using difference_type = void;
  using pointer = T *;
  using reference = T &;
  using next_function = function<optional<T>()>;
  functional_iterator() = default;
  functional_iterator(const functional_iterator<T> &) = default;
  functional_iterator &operator=(const functional_iterator<T> &) = default;
  functional_iterator(functional_iterator<T> &&) = default;
  functional_iterator &operator=(functional_iterator<T> &&) = default;
  template <class Op,
            class P = enable_if_t<!is_same_v<Op, functional_iterator<T> &>>>
  functional_iterator(Op &&op)
      : next_(std::forward<Op>(op)), current_(next_()) {}
  bool operator!=(const functional_iterator &a) const {
    return (bool(current_) && bool(a.current_))
               ? count != a.count
               : bool(current_) != bool(a.current_);
  }
  functional_iterator &operator++() {
    // @note must be an issue with clang;
    // simply assigning or moving returned content without
    // reset did not change the value of assigned to optional
    current_.reset();
    current_ = next_();
    ++count;
    return *this;
  }
  T operator*() { return *current_; }

private:
  next_function next_;
  optional<T> current_ = optional<T>();
  int count = 0;
};

// if void doesnt work specialize
template <class T, class Opt = fmc::empty> struct abstract_container : Opt {
  abstract_container() = default;
  abstract_container(const abstract_container &) = default;
  abstract_container &operator=(const abstract_container &) = default;
  abstract_container(abstract_container &&) = default;
  abstract_container &operator=(abstract_container &&) = default;
  virtual ~abstract_container() {}
  using value_type = T;
  using iterator = functional_iterator<T>;
  using next_function = typename iterator::next_function;
  iterator begin() { return iterator(iterator_generator()); }
  iterator end() { return iterator(); }
  virtual next_function iterator_generator() = 0;
};

template <class Op, class T> auto opt_do(optional<T> &&v, Op &&op) {
  using R = std::decay_t<decltype(op(*v))>;
  return bool(v) ? optional<R>(op(*v)) : optional<R>();
}
} // namespace fmc

namespace std {
template <> struct hash<tuple<>> {
  hash() = default;
  using argument_type = tuple<>;
  using result_type = std::size_t;
  result_type operator()(argument_type const &obj) const { return 1; }
};
template <class A> struct hash<tuple<A>> {
  hash() = default;
  using argument_type = tuple<A>;
  using result_type = std::size_t;
  result_type operator()(argument_type const &obj) const {
    return std::hash<A>{}(get<0>(obj));
  }
};

template <class A, class B, class... Tail> struct hash<tuple<A, B, Tail...>> {
  hash() = default;
  using argument_type = tuple<A, B, Tail...>;
  using result_type = std::size_t;
  result_type operator()(argument_type const &obj) const {
    result_type const h1(std::hash<A>{}(get<0>(obj)));
    result_type const h2(std::hash<tuple<B, Tail...>>{}(apply(
        [](A a, B b, Tail... tail) {
          return tuple<B, Tail...>(std::forward<B>(b),
                                   std::forward<Tail>(tail)...);
        },
        obj)));
    return fmc_hash_combine(h1, h2);
  }
};

template <class A, class B> struct hash<pair<A, B>> {
  using argument_type = pair<A, B>;
  using result_type = std::size_t;
  result_type operator()(argument_type const &obj) const {
    result_type const h1(std::hash<A>{}(get<0>(obj)));
    result_type const h2(std::hash<B>{}(get<1>(obj)));
    return fmc_hash_combine(h1, h2);
  }
};
} // namespace std
