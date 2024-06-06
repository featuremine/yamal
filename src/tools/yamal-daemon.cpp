/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#include <tclap/CmdLine.h>

#include <fmc++/config/config.hpp>
#include <fmc++/config/serialize.hpp>
#include <fmc++/fs.hpp>
#include <fmc++/time.hpp>
#include <fmc/signals.h>
#include <fmc/time.h>

#include <ytp/version.h>
#include <ytp/yamal.h>

#include <atomic>
#include <deque>
#include <string_view>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


struct yamal_t {
  yamal_t(const yamal_t &) = delete;
  yamal_t(const std::string &name, size_t initial_sz, double &rate)
      : name_(name), rate_(rate) {
    fmc_error_t *error = nullptr;
    fd_ = fmc_fopen(name.c_str(), fmc_fmode::MODIFY, &error);
    fmc_runtime_error_unless(!error) << fmc_error_msg(error);
    std::cout << "opened file at " << name << std::endl;
    yamal_ = ytp_yamal_new_2(fd_, false, &error);
    fmc_runtime_error_unless(!error) << fmc_error_msg(error);
    prev_reserved_sz_ = reserved_size();
    prev_time_ = fmc_cur_time_ns();
    ytp_yamal_allocate(yamal_, std::max(prev_reserved_sz_, initial_sz), &error);
    fmc_runtime_error_unless(!error) << fmc_error_msg(error);
    cached_fsz_ = fsize();
  }

  ~yamal_t() {
    fmc_error_t *error = nullptr;
    if (yamal_)
      ytp_yamal_del(yamal_, &error);
    if (fd_ != -1) {
      fmc_fclose(fd_, &error);
      std::cout << "closed yamal file " << name_ << std::endl;
    }
  }

  size_t reserved_size() {
    fmc_error_t *error = nullptr;
    auto sz = ytp_yamal_reserved_size(yamal_, &error);
    fmc_runtime_error_unless(!error) << fmc_error_msg(error);
    return sz;
  }

  size_t fsize() {
    fmc_error_t *error = nullptr;
    auto sz = fmc_fsize(fd_, &error);
    fmc_runtime_error_unless(!error) << fmc_error_msg(error);
    return sz;
  }

  void update_rate() {
    auto sz = reserved_size();
    auto delta_sz = sz - prev_reserved_sz_;
    auto now = fmc_cur_time_ns();
    auto delta_time = now - prev_time_;
    if (delta_time <= 0)
      return;
    prev_reserved_sz_ = sz;
    prev_time_ = now;
    rate_ = std::max(rate_, double(delta_sz) / double(delta_time));
  }

  void maybe_allocate() {
    fmc_error_t *error = nullptr;
    double projected = rate_ * 1000000000.0 + prev_reserved_sz_;
    if (projected < cached_fsz_)
      return;
    size_t desired = ceill(2.0 * rate_ * 1000000000.0 + prev_reserved_sz_);
    ytp_yamal_allocate(yamal_, desired, &error);
    cached_fsz_ = fsize();
    fmc_runtime_error_unless(!error) << fmc_error_msg(error);
  }

  fmc_fd fd_ = -1;
  ytp_yamal_t *yamal_ = nullptr;

  std::string name_;
  double &rate_;
  size_t cached_fsz_ = 0;
  int64_t prev_time_ = 0;
  size_t prev_reserved_sz_ = 0;
};

struct yamal_handler_t {
  yamal_handler_t(std::string name, double rate, size_t initial_sz)
      : name_(std::move(name)), initial_sz_(initial_sz), rate_(rate) {}
  void init() {
    try {
      inst_ = std::make_unique<yamal_t>(name_, initial_sz_, rate_);
    } catch(const std::exception& e) {
      std::cerr << "unable to create yamal file " << name_ << " with error: " << e.what() << std::endl;
    }
  }
  void reset() {
    inst_.reset();
  }
  void update() {
    using namespace std;
    struct stat fdstat, nmstat;
    auto fdres = inst_ && fstat(inst_->fd_, &fdstat) == 0;
    auto nmres = stat(name_.c_str(), &nmstat) == 0;
    if (fdres && nmres && fdstat.st_dev == nmstat.st_dev && fdstat.st_ino == nmstat.st_ino)
      return;
    if (inst_) {
      inst_.reset();
    }
    if (!nmres)
      return;
    try {
      inst_ = std::make_unique<yamal_t>(name_, initial_sz_, rate_);
    } catch(const std::exception& e) {
      cerr << "unable to create yamal file " << name_ << " with error: " << e.what() << endl;
      return;
    }
  }
  operator bool() {
    return (bool)inst_;
  }
  yamal_t *operator ->() {
    return inst_.get();
  }
  std::string name_;
  size_t initial_sz_ = 0;
  double rate_ = 0;
  std::unique_ptr<yamal_t> inst_;
};

static std::atomic<bool> run = true;
static void sig_handler(int s) { run = false; }

int main(int argc, char **argv) {
  fmc_set_signal_handler(sig_handler);

  TCLAP::CmdLine cmd("ytp preempt allocation tool", ' ', YTP_VERSION);

  TCLAP::ValueArg<std::string> mainArg(
      "s", "section", "Main section to be used for the tool configuration",
      true, "main", "section");
  cmd.add(mainArg);

  TCLAP::ValueArg<std::string> cfgArg("c", "config", "Configuration path", true,
                                      "preempt.ini", "path");
  cmd.add(cfgArg);

  cmd.parse(argc, argv);

  auto cfg = fmc::configs::serialize::variant_map_load_ini_structured(
      cfgArg.getValue().c_str(), mainArg.getValue().c_str());

  std::deque<yamal_handler_t> ytps;

  for (auto &&[index, ytp] : cfg["ytps"].to_a()) {
    auto &ytp_cfg = ytp.to_d();
    auto path = ytp_cfg["path"].to_s();
    auto initial_rate =
        ytp_cfg.has("rate") ? ytp_cfg["rate"].get(fmc::typify<double>()) : 0.0;
    auto initial_size =
        ytp_cfg.has("initial_size")
            ? size_t(ytp_cfg["initial_size"].get(fmc::typify<unsigned>())) *
                  1024ull * 1024ull
            : size_t(0);
    ytps.emplace_back(path.c_str(), initial_rate, initial_size);
  }

  int64_t handler_upd_time = 0;
  int64_t rate_upd_time = 0;
  while (run.load()) {
    auto now = fmc_cur_time_ns();
    if (now > handler_upd_time + 1000000000LL) {
      handler_upd_time = now;
      for (auto &handler : ytps)
        handler.update();
    }

    if (now > rate_upd_time + 10000000LL) {
      rate_upd_time = now;
      for (auto &handler : ytps) {
        if (!handler)
          continue;
        try {
          handler->update_rate();
        } catch(const std::exception& e) {
          std::cerr << "closing yamal file " << handler.name_ << " due to exception: " << e.what() << std::endl; 
          handler.reset();
        }
      }
    }

    for (auto &handler : ytps) {
      if (!handler)
        continue;
      try {
        handler->maybe_allocate();
      } catch(const std::exception& e) {
        std::cerr << "closing yamal file " << handler.name_ << " due to exception: " << e.what() << std::endl; 
        handler.reset();
      }
    }
  }
}
