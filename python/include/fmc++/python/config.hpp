/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file config.hpp
 * @author Maxim Trokhimtchouk
 * @date 9 Oct 2017
 * @brief File contains C++ definition of the python config query implementation
 *
 * This file describes the python config query implementation
 */

#pragma once

#include <fmc++/config/config.hpp> // fmc::configs::interface::*
#include <fmc++/metatable.hpp>     // fmc::metatable
#include <fmc++/misc.hpp>          // next_function
#include <fmc++/mpl.hpp>           // fmc::iterator_mapper; fmc::typify
#include <fmc++/python/wrapper.hpp>
#include <fmc++/time.hpp> // fmc::time

#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::pair

namespace fmc {
namespace python {
namespace configs {

struct node;
struct section;
struct array;

struct node : fmc::configs::interface::node {
private:
  fmc::python::object obj_;
  std::unique_ptr<section> section_;
  std::unique_ptr<array> array_;

public:
  node(fmc::python::object &&obj);
  node() = default;
  node(const node &) = delete;
  node(node &&) = default;
  node &operator=(node &&) = default;

  node &operator[](std::string_view key);

  unsigned get(typify<unsigned>);
  int get(typify<int>);
  long get(typify<long>);
  bool get(typify<bool>);
  double get(typify<double>);
  fmc::time get(typify<fmc::time>);
  std::string get(typify<std::string>);
  fmc::python::object get(typify<fmc::python::object>);

  section &get(typify<section &>);
  section &get(typify<section>);
  fmc::configs::interface::section &
      get(typify<fmc::configs::interface::section &>);

  array &get(typify<array &>);
  array &get(typify<array>);
  fmc::configs::interface::array &get(typify<fmc::configs::interface::array &>);

  section &to_d();
  array &to_a();

  template <class T> T get();
  std::string str() const;
  fmc::configs::interface::node::_type type();

  template <class T> auto as() -> decltype(get(typify<T>())) {
    return get(typify<T>());
  }
};

struct section : fmc::configs::interface::section {
private:
  struct _mapping {
    _mapping() = default;
    std::pair<std::string, fmc::configs::interface::node &>
    operator()(std::pair<std::string, fmc::configs::interface::node &> in);
  };
  fmc::python::object obj_;
  fmc::metatable<std::string, node> table_;

public:
  section(fmc::python::object &&obj);
  section(const section &) = default;
  section &operator=(const section &) = default;
  section(section &&) = default;
  section &operator=(section &&) = default;

  node &operator[](std::string_view key);

  node &get(std::string_view key);

  bool has(std::string_view key);

  using value_type = std::pair<std::string, node &>;
  using section_next_function = next_function;
  section_next_function iterator_generator();

  using iterator =
      fmc::iterator_mapper<fmc::configs::interface::section::iterator,
                           _mapping>;

  iterator begin() {
    return iterator(fmc::configs::interface::section::begin());
  }
  iterator end() { return iterator(fmc::configs::interface::section::end()); }
};

struct array : fmc::configs::interface::array {
private:
  fmc::python::object obj_;
  fmc::metatable<size_t, node> table_;
  struct _mapping {
    _mapping() = default;
    std::pair<size_t, fmc::configs::interface::node &>
    operator()(std::pair<size_t, fmc::configs::interface::node &> in);
  };

public:
  array(fmc::python::object &&obj);
  array(const array &) = default;
  array &operator=(const array &) = default;
  array(array &&) = default;
  array &operator=(array &&) = default;

  node &operator[](size_t key);

  node &get(size_t key);

  using value_type = std::pair<size_t, node &>;
  using array_next_function = next_function;
  array_next_function iterator_generator();
  using iterator =
      fmc::iterator_mapper<fmc::configs::interface::array::iterator, _mapping>;

  iterator begin() { return iterator(fmc::configs::interface::array::begin()); }
  iterator end() { return iterator(fmc::configs::interface::array::end()); }
};

using instance = section;

} // namespace configs
} // namespace python
} // namespace fmc
