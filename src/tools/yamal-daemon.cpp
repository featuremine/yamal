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
  yamal_t(const std::string &name, double rate, size_t initial_sz)
      : initial_sz_(initial_sz), rate_(rate) {
    fd_ = fmc_fopen(name.c_str(), fmc_fmode::MODIFY, &error_);
    fmc_runtime_error_unless(!error_) << fmc_error_msg(error_);
    yamal_ = ytp_yamal_new_2(fd_, false, &error_);
    fmc_runtime_error_unless(!error_) << fmc_error_msg(error_);
    prev_sz_ = size();
    prev_time_ = fmc_cur_time_ns();
    allocate(std::max(prev_sz_, initial_sz_));
    file_size_ = fsize();
  }

  void update_rate() {
    auto sz = size();
    auto delta_sz = sz - prev_sz_;
    prev_sz_ = sz;
    auto now = fmc_cur_time_ns();
    auto delta_time = now - prev_time_;
    prev_time_ = now;
    rate_ = std::max(rate_, double(delta_sz) / double(delta_time));
  }

  ~yamal_t() {
    if (yamal_)
      ytp_yamal_del(yamal_, &error_);
    if (fd_ != -1)
      fmc_fclose(fd_, &error_);
  }

  void allocate(size_t sz_required, fmc_error_t **error) {
    auto allocated_pages =
        (file_size_ + YTP_MMLIST_PAGE_SIZE - 1) / YTP_MMLIST_PAGE_SIZE;
    auto required_pages =
        (sz_required + YTP_MMLIST_PAGE_SIZE - 1) / YTP_MMLIST_PAGE_SIZE;
    ytp_yamal_allocate_pages(yamal_, allocated_pages, required_pages, error);
  }

  size_t size() {
    auto sz = ytp_yamal_reserved_size(yamal_, &error_);
    fmc_runtime_error_unless(!error_)
        << "Unable to get reserved size: " << fmc_error_msg(error_);
    return sz;
  }

  size_t fsize() {
    auto sz = fmc_fsize(fd_, &error_);
    fmc_runtime_error_unless(!error_) << "Unable to get the ytp size: " << fmc_error_msg(error_);
    return sz;
  }

  fmc_error_t *error_;
  fmc_fd fd_ = -1;
  ytp_yamal_t *yamal_ = nullptr;

  size_t initial_sz_ = 0;
  size_t file_size_ = 0;
  size_t prev_time_ = 0;
  size_t prev_sz_ = 0;
  double rate_ = 0;
  size_t prev_allocs_ = 1;
};

struct yamal_ptr_t {
  yamal_ptr_t(std::string name, double rate, size_t initial_sz)
      : name_(std::move(name)), initial_sz_(initial_sz), rate_(rate) {}
  void init() {
    try {
      inst_ = std::make_unique<yamal_t>(name_, rate_, initial_sz_);
      std::cerr<<"opened file at "<<name_<<std::endl;
    } catch(const std::exception& e) {
      std::cerr << e.what() << std::endl;
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
      cout << "closed yamal file " << name_ << endl;
    }
    if (!nmres)
      return;
    try {
      inst_ = std::make_unique<yamal_t>(name_, rate_, initial_sz_);
    } catch(const std::exception& e) {
      cerr << "unable to create yamal file " << name_ << " with error: " << e.what() << endl;
      return;
    }
    cout << "opened yamal file " << name_ << endl;
  }
  operator bool() {
    return (bool)inst_;
  }
  yamal_t &operator *() {
    return *inst_.get();
  }
  std::string name_;
  size_t initial_sz_ = 0;
  double rate_ = 0;
  std::unique_ptr<yamal_t> inst_;
};

struct alloc_time_model_t {
  alloc_time_model_t(size_t alloc_time) : alloc_time_(alloc_time) {}

  int64_t compute(size_t current_sz, size_t delta_sz) { return alloc_time_; }

  void adjust(size_t before_sz, size_t after_sz, int64_t delta_time) {
    alloc_time_ = std::max(alloc_time_, delta_time);
  }

  int64_t alloc_time_;
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

  std::deque<yamal_ptr_t> ytps;

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

  alloc_time_model_t alloc_time_model(3000000);
  int sleep_count = 0;
  int64_t last_sym_upd = 0;
  while (run.load()) {
    if (sleep_count++ == 10000) {
      sleep_count = 0;
      for (auto &ytp_ptr : ytps) {
        if (!ytp_ptr)
          continue;
        auto &ytp = *ytp_ptr;
        ytp.update_rate();
      }
    }

    int64_t alloc_time = 0;
    for (auto &ytp_ptr : ytps) {
      if (!ytp_ptr)
        continue;
      auto &ytp = *ytp_ptr;
      auto current_fsz = ytp.fsize();
      alloc_time += alloc_time_model.compute(
          current_fsz, ytp.prev_allocs_ * YTP_MMLIST_PAGE_SIZE);
    }
    for (auto &ytp_ptr : ytps) {
      if (!ytp_ptr)
        continue;
      auto &ytp = *ytp_ptr;
      auto sz = ytp.size();
      auto delta_sz = size_t(ytp.rate_ * double(alloc_time));
      if (YTP_MMLIST_PREALLOC_SIZE + (delta_sz * 11 / 100) + sz +
              (YTP_MMLIST_PAGE_SIZE * 5 / 100) >
          ytp.file_size_) {
        sleep_count = 0;
        auto before_time = fmc_cur_time_ns();
        ytp.allocate(ytp.file_size_ + YTP_MMLIST_PAGE_SIZE);
        auto after_time = fmc_cur_time_ns();
        auto delta_time = after_time - before_time;
        alloc_time_model.adjust(ytp.file_size_, YTP_MMLIST_PAGE_SIZE,
                                delta_time);
        ytp.file_size_ += YTP_MMLIST_PAGE_SIZE;
        std::cout << fmc::time(std::chrono::nanoseconds(fmc_cur_time_ns()))
                  << " Page allocated in " << delta_time << " ns (" << ytp.name_
                  << ")" << std::endl;
      }
    }
    auto now = fmc_cur_time_ns();
    if (now > last_sym_upd + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count()) {
      for (auto &ytp_ptr : ytps) {
        ytp_ptr.update();
      }
      last_sym_upd = now;
    }
  }
}
