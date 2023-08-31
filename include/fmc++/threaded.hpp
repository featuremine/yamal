/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file threaded.hpp
 * @author Federico Ravchina
 * @date 22 Jul 2021
 * @brief File contains C++ implementation of threaded
 *
 * This file contains C++ declaration of threaded.
 */

#pragma once

#include <atomic>
#include <memory>

namespace fmc {

template <typename T> class threaded {
public:
  template <typename... Args>
  threaded(Args &&...args) noexcept
      : slot_(nullptr), consumed_(new T(std::forward<Args>(args)...)),
        cleanup_(nullptr), cache_(*consumed_) {}

  threaded(threaded<T> &&rhs) noexcept
      : slot_(rhs.slot_.load()), consumed_(rhs.consumed_),
        cleanup_(std::move(rhs.cleanup_)), cache_(std::move(cache_)) {
    rhs.slot_ = nullptr;
    rhs.consumed_ = nullptr;
  }

  ~threaded() noexcept {
    std::unique_ptr<T>{slot_.load(std::memory_order_relaxed)};
    std::unique_ptr<T>{consumed_};
  }

  T &reserve() noexcept { return cache_; }

  void commit() noexcept { std::unique_ptr<T>{slot_.exchange(new T(cache_))}; }

  const T &consume() noexcept {
    auto *p = slot_.load();
    if (p == nullptr) {
      return *consumed_;
    }

    cleanup_ = std::unique_ptr<T>(consumed_);
    consumed_ = slot_.exchange(nullptr);
    return *consumed_;
  }

  void cleanup() noexcept { cleanup_ = nullptr; }

private:
  std::atomic<T *> slot_;
  T *consumed_;
  std::unique_ptr<T> cleanup_;
  T cache_;
};

} // namespace fmc
