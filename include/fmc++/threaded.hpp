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
  threaded(Args &&... args) noexcept
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
