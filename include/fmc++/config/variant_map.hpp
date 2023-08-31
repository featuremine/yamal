/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file variant_map.hpp
 * @date 06 Aug 2021
 * @brief File contains C++ definition of generic config
 *
 * This file describes generic config
 */

#pragma once

#include "fmc++/config/config.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/strings.hpp"
#include "fmc++/time.hpp"

#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace fmc {

namespace configs {

namespace variant_map {

struct node;

struct section : public fmc::configs::interface::section {
  section() = default;
  section(section &&) = default;
  section &operator=(section &&) = default;
  inline section(fmc::configs::interface::section &sec);
  inline fmc::configs::interface::node &get(string_view key) override;
  inline next_function iterator_generator() override;
  inline bool has(string_view key) override;
  inline node &operator[](string_view key);
  std::map<std::string, node> table_;
};

struct array : public fmc::configs::interface::array {
  array() = default;
  array(array &&) = default;
  array &operator=(array &&) = default;
  array(fmc::configs::interface::array &arr) {
    for (auto &&[idx, n] : arr) {
      table_.push_back(std::make_unique<node>(n));
    }
  }
  inline fmc::configs::interface::node &get(size_t key) override;
  inline next_function iterator_generator() override;
  inline node &operator[](size_t key);
  inline void push_back(node &&);
  std::vector<std::unique_ptr<node>> table_;
};

struct node : fmc::configs::interface::node {
  using node_t = std::variant<bool, long, unsigned, int, double, fmc::time,
                              std::string, section, array>;

  node() = default;
  node(node &&) = default;
  node &operator=(node &&) = default;

  template <class T,
            std::enable_if_t<!std::is_same_v<std::remove_cv_t<T>,
                                             fmc::configs::interface::node>,
                             bool> = true>
  node(T &&t) : val_(std::forward<T>(t)) {}

  node(fmc::configs::interface::node &n) {
    switch (n.type()) {
    case fmc::configs::interface::node::SECTION: {
      val_ = section(n.to_d());
    } break;
    case fmc::configs::interface::node::ARRAY: {
      val_ = array(n.to_a());
    } break;
    case fmc::configs::interface::node::STRING:
      val_ = n.as<std::string>();
      break;
    case fmc::configs::interface::node::TIME:
      val_ = n.as<fmc::time>();
      break;
    case fmc::configs::interface::node::FLOATING_POINT:
      val_ = n.as<double>();
      break;
    case fmc::configs::interface::node::INTEGER:
      val_ = n.as<int>();
      break;
    case fmc::configs::interface::node::UNSIGNED:
      val_ = n.as<unsigned>();
      break;
    case fmc::configs::interface::node::LONG:
      val_ = n.as<long>();
      break;
    case fmc::configs::interface::node::BOOL:
      val_ = n.as<bool>();
      break;
    default:
      fmc_runtime_error_unless(false)
          << "Unable to copy config, invalid section type";
      break;
    }
  }
  unsigned get(typify<unsigned>) override {
    auto fail = [](auto &m) -> unsigned {
      fmc_runtime_error_unless(false) << "Unable to cast node to desired value";
      return 0;
    };
    auto succeed_int = [](int &m) -> unsigned { return m; };
    auto succeed_long = [](long &m) -> unsigned { return m; };
    auto succeed_unsigned = [](unsigned &m) -> unsigned { return m; };
    auto succeed_string = [](std::string &m) -> unsigned {
      auto ret = fmc::_from_string_view_unsigned<unsigned>(m);
      fmc_runtime_error_unless(ret.second.size() == m.size())
          << "Unable to cast node to desired value";
      return ret.first;
    };
    return std::visit(fmc::overloaded{succeed_int, succeed_long,
                                      succeed_unsigned, succeed_string, fail},
                      val_);
  }
  int get(typify<int>) override {
    auto fail = [](auto &m) -> int {
      fmc_runtime_error_unless(false) << "Unable to cast node to desired value";
      return 0;
    };
    auto succeed_int = [](int &m) -> int { return m; };
    auto succeed_long = [](long &m) -> int { return m; };
    auto succeed_unsigned = [](unsigned &m) -> int { return m; };
    auto succeed_string = [](std::string &m) -> int {
      auto ret = fmc::from_string_view<int>(m);
      fmc_runtime_error_unless(ret.second.size() == m.size())
          << "Unable to cast node to desired value";
      return ret.first;
    };
    return std::visit(fmc::overloaded{succeed_int, succeed_long,
                                      succeed_unsigned, succeed_string, fail},
                      val_);
  }
  long get(typify<long>) override {
    auto fail = [](auto &m) -> long {
      fmc_runtime_error_unless(false) << "Unable to cast node to desired value";
      return 0;
    };
    auto succeed_int = [](int &m) -> long { return m; };
    auto succeed_long = [](long &m) -> long { return m; };
    auto succeed_unsigned = [](unsigned &m) -> long { return m; };
    auto succeed_string = [](std::string &m) -> long {
      auto ret = fmc::from_string_view<int64_t>(m);
      fmc_runtime_error_unless(ret.second.size() == m.size())
          << "Unable to cast node to desired value";
      return ret.first;
    };
    return std::visit(fmc::overloaded{succeed_int, succeed_long,
                                      succeed_unsigned, succeed_string, fail},
                      val_);
  }
  bool get(typify<bool>) override {
    auto fail = [](auto &m) -> bool {
      fmc_runtime_error_unless(false) << "Unable to cast node to desired value";
      return false;
    };
    auto succeed_bool = [](bool &m) -> bool { return m; };
    auto succeed_string = [](std::string &m) -> bool {
      if (m == "true") {
        return true;
      }
      fmc_runtime_error_unless(m == "false")
          << "Unable to cast node to desired value";
      return false;
    };
    return std::visit(fmc::overloaded{succeed_bool, succeed_string, fail},
                      val_);
  }
  double get(typify<double>) override {
    auto fail = [](auto &m) -> double {
      fmc_runtime_error_unless(false) << "Unable to cast node to desired value";
      return 0;
    };
    auto succeed_double = [](double &m) -> double { return m; };
    auto succeed_int = [](int &m) -> double { return m; };
    auto succeed_long = [](long &m) -> double { return m; };
    auto succeed_unsigned = [](unsigned &m) -> double { return m; };
    auto succeed_string = [](std::string &m) -> double {
      auto ret = fmc::from_string_view<double>(m);
      fmc_runtime_error_unless(ret.second.size() == m.size())
          << "Unable to cast node to desired value";
      return ret.first;
    };
    return std::visit(fmc::overloaded{succeed_double, succeed_int, succeed_long,
                                      succeed_unsigned, succeed_string, fail},
                      val_);
  }
  fmc::time get(typify<fmc::time>) override {
    auto pval = std::get_if<fmc::time>(&val_);
    fmc_runtime_error_unless(pval) << "Unable to cast node to desired value";
    return *pval;
  }
  std::string get(typify<std::string>) override {
    auto fail = [](auto &m) -> std::string {
      fmc_runtime_error_unless(false) << "Unable to cast node to desired value";
      return std::string();
    };
    auto succeed_double = [](double &m) -> std::string {
      return std::to_string(m);
    };
    auto succeed_int = [](int &m) -> std::string { return std::to_string(m); };
    auto succeed_long = [](long &m) -> std::string {
      return std::to_string(m);
    };
    auto succeed_unsigned = [](unsigned &m) -> std::string {
      return std::to_string(m);
    };
    auto succeed_string = [](std::string &m) -> std::string { return m; };
    return std::visit(fmc::overloaded{succeed_double, succeed_int, succeed_long,
                                      succeed_unsigned, succeed_string, fail},
                      val_);
  }
  fmc::configs::interface::section &
  get(typify<fmc::configs::interface::section &>) override {
    auto pval = std::get_if<section>(&val_);
    fmc_runtime_error_unless(pval) << "Unable to cast node to desired value";
    return *pval;
  }
  fmc::configs::interface::array &
  get(typify<fmc::configs::interface::array &>) override {
    auto pval = std::get_if<array>(&val_);
    fmc_runtime_error_unless(pval) << "Unable to cast node to desired value";
    return *pval;
  }
  std::string str() const override {
    if (auto pval = std::get_if<std::string>(&val_)) {
      return *pval;
    } else if (auto pval = std::get_if<long>(&val_)) {
      return std::to_string(*pval);
    } else if (auto pval = std::get_if<int>(&val_)) {
      return std::to_string(*pval);
    } else if (auto pval = std::get_if<unsigned>(&val_)) {
      return std::to_string(*pval);
    } else if (auto pval = std::get_if<bool>(&val_)) {
      if (*pval)
        return "True";
      return "False";
    } else if (auto pval = std::get_if<double>(&val_)) {
      return std::to_string(*pval);
    } else if (auto pval = std::get_if<fmc::time>(&val_)) {
      return std::to_string(*pval);
    }

    fmc_runtime_error_unless(false)
        << "Unable to generate string representation of object";

    // TODO: Implement

    return std::string();
  }

  fmc::configs::interface::node::_type type() override {
    // TODO: Migrate to visitor pattern
    if (std::get_if<std::string>(&val_)) {
      return fmc::configs::interface::node::STRING;
    } else if (std::get_if<int>(&val_)) {
      return fmc::configs::interface::node::INTEGER;
    } else if (std::get_if<long>(&val_)) {
      return fmc::configs::interface::node::LONG;
    } else if (std::get_if<unsigned>(&val_)) {
      return fmc::configs::interface::node::UNSIGNED;
    } else if (std::get_if<bool>(&val_)) {
      return fmc::configs::interface::node::BOOL;
    } else if (std::get_if<double>(&val_)) {
      return fmc::configs::interface::node::FLOATING_POINT;
    } else if (std::get_if<fmc::time>(&val_)) {
      return fmc::configs::interface::node::TIME;
    } else if (std::get_if<section>(&val_)) {
      return fmc::configs::interface::node::SECTION;
    } else if (std::get_if<array>(&val_)) {
      return fmc::configs::interface::node::ARRAY;
    }
    return fmc::configs::interface::node::INVALID;
  }
  node_t val_;
};

section::section(fmc::configs::interface::section &sec) {
  for (auto &&[key, n] : sec) {
    table_[key] = n;
  }
}

fmc::configs::interface::node &section::get(string_view key) {
  auto it = table_.find(std::string(key));
  fmc_runtime_error_unless(it != table_.end())
      << "Unable to find element in section with key " << key;
  return it->second;
}
section::next_function section::iterator_generator() {
  auto iterator = table_.begin();
  return [iterator, this]() mutable {
    if (iterator == table_.end())
      return std::optional<value_type>();
    std::string key = iterator->first;
    auto &val = iterator->second;
    ++iterator;
    return std::make_optional<value_type>(key, val);
  };
}
bool section::has(string_view key) {
  return table_.find(std::string(key)) != table_.end();
}

node &section::operator[](string_view key) {
  auto it = table_.find(std::string(key));
  if (it == table_.end()) {
    it = table_.emplace(key, node()).first;
  }
  return it->second;
}

fmc::configs::interface::node &array::get(size_t key) {
  fmc_runtime_error_unless(key < table_.size())
      << "Unable to obtain item in config array. Index out of range";
  return *table_[key].get();
}
array::next_function array::iterator_generator() {
  return [this, i = 0U]() mutable {
    if (i >= table_.size())
      return std::optional<value_type>();
    auto index = i++;
    return std::make_optional<value_type>(index, *table_[index].get());
  };
}

node &array::operator[](size_t key) {
  fmc_runtime_error_unless(key < table_.size())
      << "Unable to obtain item in config array. Index out of range";
  return *table_[key].get();
}

void array::push_back(node &&n) {
  table_.emplace_back(std::make_unique<node>(std::forward<node>(n)));
}

} // namespace variant_map

} // namespace configs

} // namespace fmc
