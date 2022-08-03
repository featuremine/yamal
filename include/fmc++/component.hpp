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
