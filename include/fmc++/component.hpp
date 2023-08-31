/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#pragma once

#include <atomic>
#include <mutex>
#include <signal.h>
#include <thread>

#include "fmc++/config/config.hpp"
#include "fmc++/config/variant_map.hpp"
#include "fmc/error.h"
#include "fmc/files.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <list>

namespace fmc {

class component {
public:
  component(std::function<void(fmc_fd, fmc::configs::interface::node &)> f,
            fmc::configs::interface::node &cfg) {

    fmc_fd pipe_descriptors[2];

    if (pipe(pipe_descriptors) < 0) {
      error_ = "Unable to create pipe";
      done_ = true;
      return;
    }

    control_fd = pipe_descriptors[1];

    thread_ =
        std::thread([this, pipe_descriptor = pipe_descriptors[0], f,
                     conf = fmc::configs::variant_map::node(cfg)]() mutable {
          try {
            f(pipe_descriptor, conf);
          } catch (const std::exception &e) {
            std::scoped_lock lock(lock_);
            error_ = e.what();
          }
          done_ = true;
          fmc_error_t *err;
          fmc_fclose(pipe_descriptor, &err);
        });
  }

  void stop() {
    if (!done_.load()) {
      fmc_write(control_fd, "\0", 1);
    }
  }

  bool alive(fmc_error_t **err) {
    fmc_error_clear(err);
    std::scoped_lock lock(lock_);
    if (!error_.empty()) {
      fmc_error_set(err, error_.c_str());
    }
    return !done_;
  }

  ~component() {
    stop();
    if (thread_.joinable()) {
      thread_.join();
    }
    if (fmc_fvalid(control_fd)) {
      fmc_error_t *err;
      fmc_fclose(control_fd, &err);
    }
  }

private:
  std::thread thread_;
  std::atomic<bool> done_ = false;
  std::mutex lock_;
  std::string error_;
  fmc_fd control_fd;
};

} // namespace fmc
