/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file fxpt128.cpp
 * @date 19 Oct 2022
 * @brief File contains tests for fxpt128
 *
 * @see http://www.featuremine.com
 */

#include "fmc/fxpt128.h"
#include "fmc++/fxpt128.hpp"
#include "fmc/error.h"
#include "fmc/math.h"
#include <fmc++/counters.hpp>
#include <fmc++/gtestwrap.hpp>

#include <fenv.h>
#include <inttypes.h>
#include <random>
#include <string.h>

#include <tuple>

// C API

TEST(fxpt128, from_to_double_frac) {
  double phi1 = 161.80339887498948482045868343656;
  fmc_fxpt128_t phi2;
  fmc_fxpt128_from_double(&phi2, phi1);

  double base = 0.0;
  fmc_fxpt128_t test = {0};

  for (uint x = 0; x < 10000000ULL; ++x) {
    fmc_fxpt128_t a;
    fmc_fxpt128_from_double(&a, base);
    ASSERT_EQ(base, fmc_fxpt128_to_double(&a));
    base += phi1;
    fmc_fxpt128_add(&test, &test, &phi2);
  }
  // verified with python
  // >>> ((161 * (2**64) + 14820093436037169152) * 10000000) / (2**64)
  // 1618033988.7498949
  ASSERT_EQ(1618033988.7498949, fmc_fxpt128_to_double(&test));
}

TEST(fxpt128, from_to_double_int) {
  double phi1 = 161803.0;
  fmc_fxpt128_t phi2;
  fmc_fxpt128_from_double(&phi2, phi1);

  double base = 0.0;
  fmc_fxpt128_t test = {0};

  for (uint x = 0; x < 10000000ULL; ++x) {
    fmc_fxpt128_t a;
    fmc_fxpt128_from_double(&a, base);
    ASSERT_EQ(base, fmc_fxpt128_to_double(&a));
    ASSERT_EQ(base, fmc_fxpt128_to_double(&test));
    base += phi1;
    fmc_fxpt128_add(&test, &test, &phi2);
  }
}

TEST(fxpt128, round) {
  auto converter = [](double x1) {
    fmc_fxpt128_t x2;
    fmc_fxpt128_from_double(&x2, x1);
    return x2;
  };

  auto largest = converter(fmc_double_make(0, 1075, 0));
  auto lbit1 = converter(1.0);
  auto gbit1 = converter(1.0 / 2.0);
  auto rbit1 = converter(1.0 / 4.0);
  auto sbit1 = converter(1.0 / 8.0);

  auto gen = [&](bool l, bool g, bool r, bool s, double base) {
    fmc_fxpt128_t x;
    fmc_fxpt128_copy(&x, &largest);
    if (l)
      fmc_fxpt128_add(&x, &x, &lbit1);
    if (g)
      fmc_fxpt128_add(&x, &x, &gbit1);
    if (r)
      fmc_fxpt128_add(&x, &x, &rbit1);
    if (s)
      fmc_fxpt128_add(&x, &x, &sbit1);
    ASSERT_EQ(fmc_fxpt128_to_double(&x), base);
  };

  gen(0, 0, 0, 0, 4503599627370496.0);
  gen(0, 0, 0, 1, 4503599627370496.0);
  gen(0, 0, 1, 0, 4503599627370496.0);
  gen(0, 0, 1, 1, 4503599627370496.0);
  gen(0, 1, 0, 0, 4503599627370496.0);
  gen(0, 1, 0, 1, 4503599627370497.0);
  gen(0, 1, 1, 0, 4503599627370497.0);
  gen(0, 1, 1, 1, 4503599627370497.0);

  gen(1, 0, 0, 0, 4503599627370497.0);
  gen(1, 0, 0, 1, 4503599627370497.0);
  gen(1, 0, 1, 0, 4503599627370497.0);
  gen(1, 0, 1, 1, 4503599627370497.0);
  gen(1, 1, 0, 0, 4503599627370498.0);
  gen(1, 1, 0, 1, 4503599627370498.0);
  gen(1, 1, 1, 0, 4503599627370498.0);
  gen(1, 1, 1, 1, 4503599627370498.0);
}

// NOTE: conversion from string to fxpt128 back to string is guarantied with up
// to 18 decimal digits
TEST(fxpt128, string_fxpt_string_fxpt) {
  double phi1 = 161803398874989.48482045868343656;
  fmc_fxpt128_t phi2;
  fmc_fxpt128_from_double(&phi2, phi1);

  double base = 0.0;
  fmc_fxpt128_t test = {0};

  for (uint x = 0; x < 6000ULL; ++x) {
    char buf[FMC_FXPT128_STR_SIZE] = {0};
    char res[FMC_FXPT128_STR_SIZE] = {0};
    uint64_t whole;
    snprintf(buf, sizeof(buf), "%.0f", base);
    sscanf(buf, "%" PRId64, &whole);
    int len = strlen(buf);
    buf[len] = '.';
    strncpy(buf + len + 1, buf, len);
    const char *end = buf + len;
    fmc_fxpt128_from_string(&test, buf, &end);
    ASSERT_EQ(whole, test.hi);

    end = nullptr;
    fmc_fxpt128_from_string(&test, buf, &end);
    ASSERT_EQ(end, buf + 2 * len + 1);
    fmc_fxpt128_format_t opt = {.precision = len, .decimal = len};
    fmc_fxpt128_to_string_opt(res, FMC_FXPT128_STR_SIZE, &test, &opt);
    ASSERT_STREQ(buf, res);

    auto test2 =
        fmc::fxpt128::from_string_view(std::string_view{res, strlen(res)});
    ASSERT_EQ(test, test2.first);
    base += phi1;
  }
}

// NOTE: conversion from string to fxpt128 back to string is guarantied with up
// to 18 decimal digits
TEST(fxpt128, double_fxpt_double) {
  double phi = 161803398.87498948482045868343656;
  double base = 0.0;
  for (uint x = 0; x < 6000000ULL; ++x) {
    int64_t whole = base;
    double a = double(whole) / 10000.0;
    double b = double(whole) / 256.0;
    ASSERT_EQ(double(fmc::fxpt128(a)), a);
    ASSERT_EQ(double(fmc::fxpt128(b)), b);
    base += phi;
  }
}

TEST(fxpt128, string_fxpt_no_trailing_zeros) {
  const char *phi1 = "45.32";
  fmc_fxpt128_t phi2;
  fmc_fxpt128_from_string(&phi2, phi1, nullptr);
  char res[FMC_FXPT128_STR_SIZE] = {0};
  fmc_fxpt128_format_t fmt{.precision = 18};
  fmc_fxpt128_to_string_opt(res, FMC_FXPT128_STR_SIZE, &phi2, &fmt);
  ASSERT_STREQ(phi1, res);
}

TEST(fxpt128, double_fxpt_no_trailing_zeros) {
  double phi1 = 45.32;
  fmc_fxpt128_t phi2;
  fmc_fxpt128_from_double(&phi2, phi1);
  char res[FMC_FXPT128_STR_SIZE] = {0};
  fmc_fxpt128_format_t fmt{.precision = 15};
  fmc_fxpt128_to_string_opt(res, FMC_FXPT128_STR_SIZE, &phi2, &fmt);
  ASSERT_STREQ("45.32", res);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
