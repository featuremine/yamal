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

struct yamal_t {
  yamal_t(const yamal_t &) = delete;
  yamal_t(std::string name, double rate, size_t initial_sz)
      : name_(std::move(name)), initial_sz_(initial_sz), rate_(rate) {
    fd_ = fmc_fopen(name_.c_str(), fmc_fmode::READWRITE, &error_);
    fmc_runtime_error_unless(!error_)
        << "Unable to open file " << name_ << ": " << fmc_error_msg(error_);

    yamal_ = ytp_yamal_new_2(fd_, false, &error_);
    fmc_runtime_error_unless(!error_)
        << "Unable to create the ytp yamal (" << name_
        << "): " << fmc_error_msg(error_);
  }

  ~yamal_t() {
    if (yamal_) {
      ytp_yamal_del(yamal_, &error_);
      fmc_runtime_error_unless(!error_)
          << "Unable to delete the ytp yamal: " << fmc_error_msg(error_);
    }

    if (fd_ != -1) {
      fmc_fclose(fd_, &error_);
      fmc_runtime_error_unless(!error_)
          << "Unable to close the file descriptor: " << fmc_error_msg(error_);
    }
  }

  void allocate(size_t sz_required) {
    auto allocated_pages =
        (file_size_ + YTP_MMLIST_PAGE_SIZE - 1) / YTP_MMLIST_PAGE_SIZE;
    auto required_pages =
        (sz_required + YTP_MMLIST_PAGE_SIZE - 1) / YTP_MMLIST_PAGE_SIZE;
    for (auto page = required_pages; page-- > allocated_pages;) {
      ytp_yamal_allocate_page(yamal_, page, &error_);
      fmc_runtime_error_unless(!error_) << "Unable to allocate page (" << name_
                                        << "): " << fmc_error_msg(error_);
    }
  }

  size_t size() {
    auto sz = ytp_yamal_reserved_size(yamal_, &error_);
    fmc_runtime_error_unless(!error_)
        << "Unable to get reserved size (" << name_
        << "): " << fmc_error_msg(error_);
    return sz;
  }

  size_t fsize() {
    auto sz = fmc_fsize(fd_, &error_);
    fmc_runtime_error_unless(!error_) << "Unable to get the ytp size (" << name_
                                      << "): " << fmc_error_msg(error_);
    return sz;
  }

  std::string name_;
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

  std::deque<yamal_t> ytps;

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

  for (auto &ytp : ytps) {
    ytp.prev_sz_ = ytp.size();
    ytp.prev_time_ = fmc_cur_time_ns();
    ytp.allocate(std::max(ytp.prev_sz_, ytp.initial_sz_));
    ytp.file_size_ = ytp.fsize();
  }

  alloc_time_model_t alloc_time_model(3000000);
  int sleep_count = 0;
  while (run.load()) {
    if (sleep_count++ == 10000) {
      sleep_count = 0;
      for (auto &ytp : ytps) {
        auto sz = ytp.size();
        auto delta_sz = sz - ytp.prev_sz_;
        ytp.prev_sz_ = sz;

        auto now = fmc_cur_time_ns();
        auto delta_time = now - ytp.prev_time_;
        ytp.prev_time_ = now;

        ytp.rate_ = std::max(ytp.rate_, double(delta_sz) / double(delta_time));
      }
    }

    int64_t alloc_time = 0;
    for (auto &ytp : ytps) {
      auto current_fsz = ytp.fsize();
      alloc_time += alloc_time_model.compute(
          current_fsz, ytp.prev_allocs_ * YTP_MMLIST_PAGE_SIZE);
    }
    for (auto &ytp : ytps) {
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
  }
}
