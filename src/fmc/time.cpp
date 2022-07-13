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
 * @file time.cpp
 * @author Maxim Trokhimtchouk
 * @date 11 Aug 2017
 * @brief File contains C implementation of time API
 *
 * @see http://www.featuremine.com
 */

#include <fmc/time.h>
#ifdef FMC_SYS_WIN
#include <iomanip>
#include <sstream>
#include <time.h>
#endif

#include <chrono>

fm_time64_t fm_time64_from_raw(int64_t value) {
  return value;
}

fm_time64_t fm_time64_from_nanos(int64_t value) {
  return fm_time64_from_raw(value);
}

fm_time64_t fm_time64_from_seconds(int32_t value) {
  return fm_time64_from_nanos(value * 1000000000ULL);
}

int64_t fm_time64_to_nanos(fm_time64_t t) { return t; }

double fm_time64_to_fseconds(fm_time64_t t) {
  return (double)t / 1000000000.0;
}

int64_t fm_time64_raw(fm_time64_t time) { return time; }

bool fm_time64_less(fm_time64_t a, fm_time64_t b) {
  return a < b;
}

bool fm_time64_greater(fm_time64_t a, fm_time64_t b) {
  return a > b;
}

bool fm_time64_equal(fm_time64_t a, fm_time64_t b) {
  return a == b;
}

int64_t fm_time64_div(fm_time64_t a, fm_time64_t b) {
  return a / b;
}

fm_time64_t fm_time64_add(fm_time64_t a, fm_time64_t b) {
  return a + b;
}

void fm_time64_inc(fm_time64_t *a, fm_time64_t b) {
  *a += b;
}

fm_time64_t fm_time64_sub(fm_time64_t a, fm_time64_t b) {
  return a - b;
}

fm_time64_t fm_time64_mul(fm_time64_t a, int64_t b) {
  return a * b;
}

fm_time64_t fm_time64_int_div(fm_time64_t a, int64_t b) {
  return a / b;
}

fm_time64_t fm_time64_start() {
  return INT64_MIN;
}

fm_time64_t fm_time64_end() {
  return INT64_MAX;
}

bool fm_time64_is_end(fm_time64_t time) {
  return time == INT64_MAX;
}

using namespace std;
using namespace chrono;

char *fmc_strptime(const char *s, const char *f, struct tm *tm) {
#if defined(FMC_SYS_WIN)
  std::istringstream input(s);
  input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
  input >> std::get_time(tm, f);
  if (input.fail()) {
    return nullptr;
  }
  return (char *)(s + input.tellg());
#elif defined(FMC_SYS_UNIX)
  return strptime(s, f, tm);
#else
#error "Unsupported operating system"
#endif
}

void fmc_gmtime(time_t *t, struct tm *tm_) {
#if defined(FMC_SYS_WIN)
  *tm_ = *gmtime(t);
#elif defined(FMC_SYS_UNIX)
  gmtime_r(t, tm_);
#else
#error "Unsupported operating system"
#endif
}

int64_t fmc_cur_time_ns() {
#ifdef FMC_SYS_WIN
  return duration_cast<chrono::nanoseconds>(
             system_clock::now().time_since_epoch())
      .count();
#elif defined(FMC_SYS_UNIX)
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (duration_cast<chrono::nanoseconds>(chrono::seconds(ts.tv_sec)) +
          duration_cast<chrono::nanoseconds>(chrono::nanoseconds(ts.tv_nsec)))
      .count();
#else
#error "Unsupported operating system"
#endif
}