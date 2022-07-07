/******************************************************************************
    COPYRIGHT (c) 2020 by Featuremine Corporation.
    This software has been provided pursuant to a License Agreement
    containing restrictions on its use.  This software contains
    valuable trade secrets and proprietary information of
    FeatureMine Corporation and is protected by law.  It may not be
    copied or distributed in any form or medium, disclosed to third
    parties, reverse engineered or used in any manner not provided
    for in said License Agreement except with the prior written
    authorization Featuremine Corporation.
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

#include "utils.hpp"

std::string time_to_string(uint64_t nanosecs) {
    time_t secs = nanosecs/1000000000;
    char       buf[64];

    // Format time, "yyyy-mm-dd hh:mm:ss.nnnnnnnnn"
    struct tm ts = *localtime(&secs);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.", &ts);
    std::ostringstream os;
    os << buf << std::setfill('0') << std::setw(9) << nanosecs % 1000000000;
    return os.str();
}

struct yamal_t {
  yamal_t(const yamal_t &) = delete;
  yamal_t(std::string name, double rate, size_t initial_sz)
      : name_(std::move(name)), rate_(rate), initial_sz_(initial_sz) {
    rv_ = ytp_initialize(); // also calls apr_initialize(), apr_pool_initialize(), apr_signal_init()
    if(rv_) {
      char error_str[128];
      throw std::runtime_error(ytp_strerror(rv_, error_str, sizeof(error_str)));
    }
    rv_ = apr_pool_create(&pool_, NULL);
    if(rv_) {
      char error_str[128];
      throw std::runtime_error(ytp_strerror(rv_, error_str, sizeof(error_str)));
    }
    rv_ = apr_file_open(&f_, name_.c_str(),
                      APR_FOPEN_CREATE | APR_FOPEN_READ,
                      APR_FPROT_OS_DEFAULT, pool_);
    if(rv_) {
      std::string err("Unable to open file ");
      err += name_;
      err += ": ";
      char error_str[128];
      err += ytp_strerror(rv_, error_str, sizeof(error_str));
      throw std::runtime_error(err);
    }

    rv_ = ytp_yamal_new2(&yamal_, f_, false);
    if(rv_) {
      std::string err("Unable to create the ytp yamal: ");
      char error_str[128];
      err += ytp_strerror(rv_, error_str, sizeof(error_str));
      throw std::runtime_error(err);
    }
  }

  ~yamal_t() {
    if(yamal_) {
      rv_ = ytp_yamal_del(yamal_);
      if(rv_) {
        std::string err("Unable to delete the ytp yamal: ");
        char error_str[128];
        err += ytp_strerror(rv_, error_str, sizeof(error_str));
        throw std::runtime_error(err);
      }
    }
    if(f_) {
      rv_ = apr_file_close(f_);
      if(rv_) {
        std::string err("Unable to close the file: ");
        char error_str[128];
        err += ytp_strerror(rv_, error_str, sizeof(error_str));
        throw std::runtime_error(err);
      }
    }
    if(pool_) {
      apr_pool_destroy(pool_);
    }
    ytp_terminate();
  }

  void allocate(size_t sz_required) {
    auto allocated_pages =
        (file_size_ + YTP_MMLIST_PAGE_SIZE - 1) / YTP_MMLIST_PAGE_SIZE;
    auto required_pages =
        (sz_required + YTP_MMLIST_PAGE_SIZE - 1) / YTP_MMLIST_PAGE_SIZE;
    for (auto page = required_pages; page-- > allocated_pages;) {
      rv_ = ytp_yamal_allocate_page(yamal_, page);
      if(rv_) {
        std::string err("Unable to allocate page (");
        err += name_;
        err += "): ";
        char error_str[128];
        err += ytp_strerror(rv_, error_str, sizeof(error_str));
        throw std::runtime_error(err);
      }
    }
  }

  size_t size() {
    apr_size_t sz;
    rv_ = ytp_yamal_reserved_size(yamal_, &sz);
    if(rv_) {
      std::string err("Unable to get reserved size (");
      err += name_;
      err += "): ";
      char error_str[128];
      err += ytp_strerror(rv_, error_str, sizeof(error_str));
      throw std::runtime_error(err);
    }
    return sz;
  }

  size_t fsize() {
    apr_finfo_t finfo;
    rv_ = apr_file_info_get(&finfo, APR_FINFO_SIZE, f_);
    if (rv_) {
      std::string err("Unable to get the ytp size (");
      err += name_;
      err += "): ";
      char error_str[128];
      err += ytp_strerror(rv_, error_str, sizeof(error_str));
      throw std::runtime_error(err);
      return 0;
    }
    return finfo.size;
  }

  std::string name_;
  ytp_status_t rv_ = YTP_STATUS_OK;
  apr_pool_t *pool_ = nullptr;
  apr_file_t *f_ = nullptr;
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
  ytp_status_t rv = ytp_initialize(); // also calls apr_initialize(), apr_pool_initialize(), apr_signal_init()
  if(rv) {
    char error_str[128];
    std::cerr << ytp_strerror(rv, error_str, sizeof(error_str)) << std::endl;
    return rv;
  }
  apr_signal(SIGQUIT, sig_handler);
  apr_signal(SIGABRT, sig_handler);
  apr_signal(SIGTERM, sig_handler);
  apr_signal(SIGINT, sig_handler);

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
    ytp.prev_time_ = cur_time_ns();
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

        auto now = cur_time_ns();
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
        auto before_time = cur_time_ns();
        ytp.allocate(ytp.file_size_ + YTP_MMLIST_PAGE_SIZE);
        auto after_time = cur_time_ns();
        auto delta_time = after_time - before_time;
        alloc_time_model.adjust(ytp.file_size_, YTP_MMLIST_PAGE_SIZE,
                                delta_time);
        ytp.file_size_ += YTP_MMLIST_PAGE_SIZE;
        std::cout << time_to_string(cur_time_ns())
                  << " Page allocated in " << delta_time << " ns (" << ytp.name_
                  << ")" << std::endl;
      }
    }
  }
  ytp_terminate();
}
