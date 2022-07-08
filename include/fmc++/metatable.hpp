/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
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
 * @file metatable.hpp
 * @author Maxim Trokhimtchouk
 * @date 6 Oct 2017
 * @brief File contains metatable declaration
 *
 * This file contains implementation of metatable class
 */

#pragma once

#include "fmc++/misc.hpp"
#include <functional>
#include <iostream>
#include <unordered_map>

#include <fmc++/mpl.hpp>
#include <iterator>
namespace fmc {

/**
 * @brief metatable class
 *
 * Acts as a storage device to keep track of items by key.
 * Items are stored in a declarative way, one must ask the metatable for
 * the desired item to ensure a bucket for it is created.
 */
template <class K, class Obj, class... Args> class metatable {
public:
  using creator_t = std::function<Obj *(const K &, Args &&...)>;
  using map_t = std::unordered_map<K, Obj *>;
  using iterator =
      fmc::iterator_mapper<typename map_t::iterator, fmc::dereference_second>;
  using const_iterator = fmc::iterator_mapper<typename map_t::const_iterator,
                                              fmc::dereference_second>;

  template <class Op,
            typename = std::enable_if_t<!std::is_same<
                std::decay_t<Op>, fmc::metatable<K, Obj, Args...>>::value>>
  metatable(Op &&op) : creator_(std::forward<Op>(op)) {}
  ~metatable() {
    for (auto &item : table_)
      delete item.second;
  }
  template <class Key> Obj &operator()(Key key, Args &&... args) {
    auto where = table_.find(std::forward<Key>(key));
    if (where == table_.end()) {
      auto *obj = creator_(std::forward<Key>(key), std::forward<Args>(args)...);
      where = table_.emplace(std::forward<Key>(key), obj).first;
    }
    return *where->second;
  }
  /**
   * @brief begin method
   *
   * Method to obtain an iterator pointing to the metatable start.
   *
   * @return iterator pointing to the start of the metatable.
   */
  iterator begin() { return table_.begin(); };
  /**
   * @brief end method
   *
   * Method to obtain an iterator pointing to the metatable end.
   *
   * @return iterator pointing to the end of the metatable.
   */
  iterator end() { return table_.end(); };
  /**
   * @brief find method
   *
   * Method to get an element from the metatable using the key used to
   * store the element as input.
   *
   * @return iterator pointing to the desired element in the metatable.
   */
  iterator find(K key) { return table_.find(key); }
  /**
   * @brief const begin method
   *
   * Constant alternative to begin method.
   *
   * @return constant iterator pointing to the start of the metatable.
   */
  const_iterator begin() const { return table_.begin(); };
  /**
   * @brief const end method
   *
   * Constant alternative to end method.
   *
   * @return constant iterator pointing to the end of the metatable.
   */
  const_iterator end() const { return table_.end(); };
  /**
   * @brief const find method
   *
   * Constant alternative to find method.
   *
   * @return constant iterator pointing to the desired element in the
   * metatable.
   */
  const_iterator find(K key) const { return table_.find(key); }
  /**
   * @brief size method
   *
   * Method to obtain the size of the metatable.
   *
   * @return the size of the metatable.
   */
  std::size_t size() const noexcept { return table_.size(); }

private:
  map_t table_;
  creator_t creator_;
};
} // namespace fmc
