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
 * @file config.hpp
 * @author Maxim Trokhimtchouk
 * @date 9 Oct 2017
 * @brief File contains C++ definition of config query interface
 *
 * This file describes config query interface
 */

#pragma once

#include "fmc++/misc.hpp" // fmc::abstract_container
#include "fmc++/time.hpp" // fmc::time
#include <fmc++/mpl.hpp>  // fmc::typify

#include <ostream>     // std::ostream
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::pair

namespace fmc {
namespace configs {
namespace interface {

struct node;
struct section;
struct array;

struct node {

  enum _type {
    INVALID,
    BOOL,
    INTEGER,
    UNSIGNED,
    LONG,
    FLOATING_POINT,
    TIME,
    STRING,
    SECTION,
    ARRAY
  };

  // TODO: Fix this
  node() = default;
  node(const node &) = default;
  node &operator=(const node &) = default;
  node(node &&) = default;
  node &operator=(node &&) = default;

  node &operator[](std::string_view key);
  friend std::ostream &operator<<(std::ostream &s, const node &n);

  virtual ~node() {}
  virtual long get(typify<long>) = 0;
  virtual unsigned get(typify<unsigned>) = 0;
  virtual int get(typify<int>) = 0;
  virtual bool get(typify<bool>) = 0;
  virtual double get(typify<double>) = 0;
  virtual fmc::time get(typify<fmc::time>) = 0;
  virtual string get(typify<std::string>) = 0;
  virtual section &get(typify<section &>) = 0;
  virtual array &get(typify<array &>) = 0;
  array &get(typify<array>);
  section &get(typify<section>);
  virtual std::string str() const = 0;
  template <class T> auto as() -> decltype(get(typify<T>())) {
    return get(typify<T>());
  }
  template <class T> explicit operator T() { return as<T>(); }
  std::string to_s();
  array &to_a();
  section &to_d();
  virtual _type type() = 0;
  bool has(std::string_view key);
};

struct section : fmc::abstract_container<std::pair<std::string, node &>> {
  virtual ~section() {}
  section() = default;
  section(const section &) = default;
  section &operator=(const section &) = default;
  section(section &&) = default;
  section &operator=(section &&) = default;
  virtual node &get(std::string_view key) = 0;
  node &operator[](std::string_view key);
  virtual bool has(std::string_view) = 0;
};

struct array : fmc::abstract_container<std::pair<size_t, node &>> {
  virtual ~array() {}
  array() = default;
  array(const array &) = default;
  array &operator=(const array &) = default;
  array(array &&) = default;
  array &operator=(array &&) = default;
  virtual node &get(size_t key) = 0;
  node &operator[](size_t key);
};

} // namespace interface
} // namespace configs
} // namespace fmc
