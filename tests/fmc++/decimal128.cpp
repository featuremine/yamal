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
 * @file decimal128.cpp
 * @date 19 Oct 2022
 * @brief File contains tests for decimal128
 *
 * @see http://www.featuremine.com
 */

#include "fmc/decimal128.h"
#include "fmc++/decimal128.hpp"
#include "fmc/error.h"
#include <fenv.h>
#include <fmc++/gtestwrap.hpp>
#include <libdecnumber/decQuad.h>
#include <random>
#include <string.h>

static_assert(sizeof(decQuad) == sizeof(fmc_decimal128_t),
              "sizeof doesn't match");
static_assert(alignof(decQuad) == alignof(fmc_decimal128_t),
              "alignof doesn't match");

// C API
TEST(decimal128, from_to_flt_str) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  std::string strval = "11111211111.111111111114111111";
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, strval.c_str());
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, bad_str) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;

  // Non numerical string
  std::string strval = "thisisnotanumber";
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);

  // Invalid numerical string
  strval = "1.2.3";
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = "-1.2.3";
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = "1.2E3E4";
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = "1.2E-3E-4";
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);

  // Numerical string length
  strval = std::string(34, '1'); // supports up to 34 decimal digits
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  strval = std::string("-") + std::string(34, '1');
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  strval = std::string("1.") + std::string(33, '1') + std::string("E+6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  strval = std::string("1.") + std::string(33, '1') + std::string("E-6143");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  strval = std::string("-1.") + std::string(33, '1') + std::string("E+6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  strval = std::string("-1.") + std::string(33, '1') + std::string("E-6143");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  strval = std::string(35, '1'); // supports up to 34 decimal digits
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = std::string("-") + std::string(35, '1');
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = std::string("1.") + std::string(33, '1') + std::string("E+6145");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = std::string("1.") + std::string(33, '1') + std::string("E-6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = std::string("-1.") + std::string(33, '1') + std::string("E+6145");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  strval = std::string("-1.") + std::string(33, '1') + std::string("E-6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
}

TEST(decimal128, from_uint_zero) {
  feclearexcept(FE_ALL_EXCEPT);
  uint64_t max = 0;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_uint(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "0");
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, to_uint_neg) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = -4;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "-4");
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_NE(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_INVALID));
}

TEST(decimal128, to_uint_out_of_range) {
  feclearexcept(FE_ALL_EXCEPT);
  uint64_t max = std::numeric_limits<uint64_t>::max();
  fmc_decimal128_t a;
  fmc_decimal128_from_uint(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_pow10(&a, 1);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  uint64_t res;
  fmc_error_t *err;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_NE(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_INVALID));
}

TEST(decimal128, from_uint_low) {
  feclearexcept(FE_ALL_EXCEPT);
  uint64_t max = 15;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_uint(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "15");
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, from_uint_extreme) {
  feclearexcept(FE_ALL_EXCEPT);
  uint64_t max = std::numeric_limits<uint64_t>::max();
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_uint(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "18446744073709551615");
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, from_int_zero) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = 0;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "0");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, from_int_low) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = 15;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "15");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, from_int_extreme) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = std::numeric_limits<int64_t>::max();
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "9223372036854775807");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, from_int_neg_low) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = -15;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "-15");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, from_int_neg_extreme) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = std::numeric_limits<int64_t>::min();
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_STREQ(str, "-9223372036854775808");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, to_int_out_of_range) {
  feclearexcept(FE_ALL_EXCEPT);
  int64_t max = std::numeric_limits<int64_t>::max();
  fmc_decimal128_t a;
  fmc_decimal128_from_int(&a, max);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_pow10(&a, 1);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  int64_t res;
  fmc_error_t *err;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_TRUE(fetestexcept(FE_INVALID));
  ASSERT_NE(err, nullptr);
}

TEST(decimal128, from_to_int_str) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  std::string strval = "5";
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_EQ(err, nullptr);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, strval.c_str());
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, divide) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "66666666666.66666666666666666", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "2.2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_div(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "30303030303.0303030303030303");
}

TEST(decimal128, divide_by_zero) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "66666666666.66666666666666666", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "0", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_div(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_DIVBYZERO));
}

TEST(decimal128, int_divide_by_zero) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "66666666666.66666666666666666", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_int_div(&c, &a, 0);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_DIVBYZERO));
}

TEST(decimal128, intdivide) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t cppa;
  fmc_decimal128_from_int(&cppa, 10);
  int64_t b = 5;
  fmc_decimal128_t cppc;
  fmc_decimal128_from_int(&cppc, 2);
  ASSERT_EQ(cppa / b, cppc);
}

TEST(decimal128, add) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "22222222222.22222222222222222", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_add(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "33333333333.33333333333333333");
}

TEST(decimal128, add_out_of_range) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "9.999999999999999999999999999999999E+6144",
                          &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "9.999999999999999999999999999999999E+6144",
                          &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_add(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_OVERFLOW));
}

TEST(decimal128, sub) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "22222222222.22222222222222222", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_sub(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "-11111111111.11111111111111111");
}

TEST(decimal128, sub_out_of_range) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "-9.999999999999999999999999999999999E+6144",
                          &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "9.999999999999999999999999999999999E+6144",
                          &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_sub(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_OVERFLOW));
}

TEST(decimal128, mul) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "2.2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_mul(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "24444444444.444444444444444442");
}

TEST(decimal128, mul_out_of_range) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "9.999999999999999999999999999999999E+6144",
                          &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "9.999999999999999999999999999999999E+6144",
                          &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_mul(&c, &a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_OVERFLOW));
}

TEST(decimal128, comparison) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "2.5", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&c, "3", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  ASSERT_FALSE(fmc_decimal128_less(&b, &a));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_less(&b, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_less(&b, &c));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  ASSERT_FALSE(fmc_decimal128_less_or_equal(&b, &a));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_less_or_equal(&b, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_less_or_equal(&b, &c));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  ASSERT_TRUE(fmc_decimal128_greater(&b, &a));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_greater(&b, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_greater(&b, &c));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  ASSERT_TRUE(fmc_decimal128_greater_or_equal(&b, &a));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_greater_or_equal(&b, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_greater_or_equal(&b, &c));

  ASSERT_FALSE(fmc_decimal128_equal(&b, &a));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&b, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_equal(&b, &c));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, round) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b, c, d, e;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "2.2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&c, "2.5", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&d, "2.7", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&e, "3", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_t ar, br, cr, dr;
  fmc_decimal128_round(&ar, &a, 0);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_round(&br, &b, 0);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_round(&cr, &c, 0);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_round(&dr, &d, 0);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fetestexcept(FE_INEXACT));
  feclearexcept(FE_ALL_EXCEPT);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &ar));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&a, &br));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&e, &cr));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&e, &dr));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, round_double) {
  char number_str[FMC_DECIMAL128_STR_SIZE];
  auto roundit = [&](double n) {
    fmc_decimal128_t a;
    fmc_decimal128_from_double(&a, n);
    fmc_decimal128_to_str(number_str, &a);
    int digits10 = fmc_decimal128_flog10abs(&a);
    fmc_decimal128_round(&a, &a, digits10 != INT32_MIN ? 14 - digits10 : 0);
    fmc_decimal128_to_str(number_str, &a);
    return std::string_view(number_str);
  };

  EXPECT_EQ(roundit(0.0), "0");
  EXPECT_EQ(roundit(0.000001), "0.000001");
  EXPECT_EQ(roundit(0.0000000008), "8E-10");
  EXPECT_EQ(roundit(0.17), "0.17");
  EXPECT_EQ(roundit(63.33), "63.33");
  EXPECT_EQ(roundit(50.55), "50.55");
  EXPECT_EQ(roundit(99.999), "99.999");
  EXPECT_EQ(roundit(123.0), "123");
  EXPECT_EQ(roundit(677.25), "677.25");
  EXPECT_EQ(roundit(45678.1), "45678.1");
  EXPECT_EQ(roundit(99999.999), "99999.999");
  EXPECT_EQ(roundit(40000.0001), "40000.0001");
  EXPECT_EQ(roundit(-645.43), "-645.43");
  EXPECT_EQ(roundit(-156.99), "-156.99");
  EXPECT_EQ(roundit(-0.0000000000123456789012345), "-1.23456789012345E-11");
  EXPECT_EQ(roundit(0.000000000123456789012345), "1.23456789012345E-10");
  EXPECT_EQ(roundit(-0.00000000123456789012345), "-1.23456789012345E-9");
  EXPECT_EQ(roundit(0.0000000123456789012345), "1.23456789012345E-8");
  EXPECT_EQ(roundit(0.000000123456789012345), "1.23456789012345E-7");
  EXPECT_EQ(roundit(-0.00000123456789012345), "-0.00000123456789012345");
  EXPECT_EQ(roundit(0.0000123456789012345), "0.0000123456789012345");
  EXPECT_EQ(roundit(0.000123456789012345), "0.000123456789012345");
  EXPECT_EQ(roundit(-0.00123456789012345), "-0.00123456789012345");
  EXPECT_EQ(roundit(0.0123456789012345), "0.0123456789012345");
  EXPECT_EQ(roundit(0.123456789012345), "0.123456789012345");
  EXPECT_EQ(roundit(-1.23456789012345), "-1.23456789012345");
  EXPECT_EQ(roundit(12.3456789012345), "12.3456789012345");
  EXPECT_EQ(roundit(-123.456789012345), "-123.456789012345");
  EXPECT_EQ(roundit(1234.56789012345), "1234.56789012345");
  EXPECT_EQ(roundit(-12345.6789012345), "-12345.6789012345");
  EXPECT_EQ(roundit(123456.789012345), "123456.789012345");
  EXPECT_EQ(roundit(-1234567.89012345), "-1234567.89012345");
  EXPECT_EQ(roundit(12345678.9012345), "12345678.9012345");
  EXPECT_EQ(roundit(-123456789.012345), "-123456789.012345");
  EXPECT_EQ(roundit(1234567890.12345), "1234567890.12345");
  EXPECT_EQ(roundit(-12345678901.2345), "-12345678901.2345");
  EXPECT_EQ(roundit(123456789012.345), "123456789012.345");
  EXPECT_EQ(roundit(-1234567890123.45), "-1234567890123.45");
  EXPECT_EQ(roundit(123456789012345.0), "123456789012345");
  EXPECT_EQ(roundit(1234567890123456789.0), "1.23456789012346E+18");
  EXPECT_EQ(roundit(std::numeric_limits<double>::infinity()), "inf");
  EXPECT_EQ(roundit(-std::numeric_limits<double>::infinity()), "-inf");
  EXPECT_EQ(roundit(std::numeric_limits<double>::quiet_NaN()), "nan");
  EXPECT_EQ(roundit(-std::numeric_limits<double>::quiet_NaN()), "-nan");
}

TEST(decimal128, increment) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_inc(&a, &a);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&a, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, decrement) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a, b;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_from_str(&b, "2", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_dec(&a, &b);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&a, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, negate) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_t b;
  fmc_decimal128_negate(&b, &a);
  ASSERT_FALSE(fmc_decimal128_equal(&a, &b));

  fmc_decimal128_t c;
  fmc_decimal128_negate(&c, &b);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &c));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, pow10) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_pow10(&a, 3);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_t b;
  fmc_decimal128_from_str(&b, "4000", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  ASSERT_TRUE(fmc_decimal128_equal(&a, &b));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_t c;
  fmc_decimal128_from_str(&c, "40", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_pow10(&a, -2);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&a, &c));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc_decimal128_t d;
  fmc_decimal128_from_str(&d, "0.04", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  fmc_decimal128_pow10(&a, -3);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&a, &d));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, infinity) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_is_inf(&a));

  fmc_decimal128_t inf;
  fmc_decimal128_inf(&inf);
  ASSERT_TRUE(fmc_decimal128_is_inf(&inf));
}

TEST(decimal128, nan) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_FALSE(fmc_decimal128_is_nan(&a));
  ASSERT_FALSE(fmc_decimal128_is_qnan(&a));
  ASSERT_FALSE(fmc_decimal128_is_snan(&a));

  fmc_decimal128_t qnan;
  fmc_decimal128_qnan(&qnan);
  ASSERT_TRUE(fmc_decimal128_is_nan(&qnan));
  ASSERT_TRUE(fmc_decimal128_is_qnan(&qnan));
  ASSERT_FALSE(fmc_decimal128_is_snan(&qnan));
  ASSERT_FALSE(fmc_decimal128_is_inf(&qnan));

  fmc_decimal128_t snan;
  fmc_decimal128_snan(&snan);
  ASSERT_TRUE(fmc_decimal128_is_nan(&snan));
  ASSERT_FALSE(fmc_decimal128_is_qnan(&snan));
  ASSERT_TRUE(fmc_decimal128_is_snan(&snan));
  ASSERT_FALSE(fmc_decimal128_is_inf(&snan));
}

TEST(decimal128, max) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_decimal128_max(&a);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, min) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_decimal128_min(&a);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "-9.999999999999999999999999999999999E+6144");
}

// C++ API
TEST(decimal128, cppconstructor) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "5", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc::decimal128 ppa(5);
  fmc::decimal128 ppb(a);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &ppa));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
  ASSERT_TRUE(fmc_decimal128_equal(&a, &ppb));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

  fmc::decimal128 ppc;
  ppa = ppa - ppa;
  ASSERT_TRUE(fmc_decimal128_equal(&ppa, &ppc));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
}

TEST(decimal128, cppdivide) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(10);
  fmc::decimal128 cppb(5);
  fmc::decimal128 cppc(2);
  ASSERT_EQ(cppa / cppb, cppc);

  fmc_decimal128_t ppa;
  fmc_decimal128_from_int(&ppa, 10);
  fmc_decimal128_t ppb;
  fmc_decimal128_from_int(&ppb, 5);
  fmc_decimal128_t ppc;
  fmc_decimal128_from_int(&ppc, 2);
  ASSERT_EQ(ppa / ppb, ppc);
}

TEST(decimal128, cppintdivide) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(10);
  int64_t b = 5;
  fmc::decimal128 cppc(2);
  ASSERT_EQ(cppa / b, cppc);
}

TEST(decimal128, cppadd) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(7);
  fmc::decimal128 cppb(5);
  fmc::decimal128 cppc(2);
  ASSERT_EQ(cppc + cppb, cppa);

  fmc_decimal128_t ppa;
  fmc_decimal128_from_int(&ppa, 7);
  fmc_decimal128_t ppb;
  fmc_decimal128_from_int(&ppb, 5);
  fmc_decimal128_t ppc;
  fmc_decimal128_from_int(&ppc, 2);
  ASSERT_EQ(ppc + ppb, ppa);
}

TEST(decimal128, cppsub) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(7);
  fmc::decimal128 cppb(5);
  fmc::decimal128 cppc(2);
  ASSERT_EQ(cppa - cppb, cppc);

  fmc_decimal128_t ppa;
  fmc_decimal128_from_int(&ppa, 7);
  fmc_decimal128_t ppb;
  fmc_decimal128_from_int(&ppb, 5);
  fmc_decimal128_t ppc;
  fmc_decimal128_from_int(&ppc, 2);
  ASSERT_EQ(ppa - ppb, ppc);
}

TEST(decimal128, cppmul) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(10);
  fmc::decimal128 cppb(5);
  fmc::decimal128 cppc(2);
  ASSERT_EQ(cppc * cppb, cppa);

  fmc_decimal128_t ppa;
  fmc_decimal128_from_int(&ppa, 10);
  fmc_decimal128_t ppb;
  fmc_decimal128_from_int(&ppb, 5);
  fmc_decimal128_t ppc;
  fmc_decimal128_from_int(&ppc, 2);
  ASSERT_EQ(ppc * ppb, ppa);
}

TEST(decimal128, cppcomparison) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(5);
  fmc::decimal128 cppb(8);
  fmc::decimal128 cppc(10);

  ASSERT_FALSE(cppb < cppa);
  ASSERT_FALSE(cppb < cppb);
  ASSERT_TRUE(cppb < cppc);

  ASSERT_FALSE(cppb <= cppa);
  ASSERT_TRUE(cppb <= cppb);
  ASSERT_TRUE(cppb <= cppc);

  ASSERT_TRUE(cppb > cppa);
  ASSERT_FALSE(cppb > cppb);
  ASSERT_FALSE(cppb > cppc);

  ASSERT_TRUE(cppb >= cppa);
  ASSERT_TRUE(cppb >= cppb);
  ASSERT_FALSE(cppb >= cppc);

  ASSERT_FALSE(cppb == cppa);
  ASSERT_TRUE(cppb == cppb);
  ASSERT_FALSE(cppb == cppc);

  ASSERT_TRUE(cppb != cppa);
  ASSERT_FALSE(cppb != cppb);
  ASSERT_TRUE(cppb != cppc);

  fmc_decimal128_t ppa;
  fmc_decimal128_from_int(&ppa, 5);
  fmc_decimal128_t ppb;
  fmc_decimal128_from_int(&ppb, 8);
  fmc_decimal128_t ppc;
  fmc_decimal128_from_int(&ppc, 10);

  ASSERT_FALSE(ppb < ppa);
  ASSERT_FALSE(ppb < ppb);
  ASSERT_TRUE(ppb < ppc);

  ASSERT_FALSE(ppb <= ppa);
  ASSERT_TRUE(ppb <= ppb);
  ASSERT_TRUE(ppb <= ppc);

  ASSERT_TRUE(ppb > ppa);
  ASSERT_FALSE(ppb > ppb);
  ASSERT_FALSE(ppb > ppc);

  ASSERT_TRUE(ppb >= ppa);
  ASSERT_TRUE(ppb >= ppb);
  ASSERT_FALSE(ppb >= ppc);

  ASSERT_FALSE(ppb == ppa);
  ASSERT_TRUE(ppb == ppb);
  ASSERT_FALSE(ppb == ppc);

  ASSERT_TRUE(ppb != ppa);
  ASSERT_FALSE(ppb != ppb);
  ASSERT_TRUE(ppb != ppc);
}

TEST(decimal128, cppincrement) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(5);
  fmc::decimal128 cppb(10);
  cppa += cppa;
  ASSERT_EQ(cppa, cppb);
}

TEST(decimal128, cppdecrement) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 cppa(10);
  fmc::decimal128 cppb(5);
  cppa -= cppb;
  ASSERT_EQ(cppa, cppb);
}

TEST(decimal128, cppupcasting) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t a;
  fmc::decimal128 &ppa = fmc::decimal128::upcast(a);
  ASSERT_EQ(&a, &ppa);
}

TEST(decimal128, cppimplicit_downcasting) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 ppa(5);
  auto f = [](const fmc::decimal128 &lhs, const fmc_decimal128_t &rhs) -> bool {
    return &lhs == &rhs;
  };
  ASSERT_TRUE(f(ppa, ppa));
}

TEST(decimal128, cppnegate) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a(4);
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));
  ASSERT_TRUE(std::isfinite(a));

  fmc::decimal128 b = -a;

  ASSERT_NE(a, b);
  ASSERT_EQ(a, -b);
}

TEST(decimal128, cppinfinity) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 ca(4);
  ASSERT_FALSE(std::isinf(ca));
  ASSERT_FALSE(std::isnan(ca));
  ASSERT_TRUE(std::isfinite(ca));
  fmc_decimal128_t a = ca;
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));
  ASSERT_TRUE(std::isfinite(a));

  fmc::decimal128 cinf = std::numeric_limits<fmc::decimal128>::infinity();
  ASSERT_TRUE(std::isinf(cinf));
  ASSERT_FALSE(std::isnan(cinf));
  ASSERT_FALSE(std::isfinite(cinf));
  fmc_decimal128_t inf = cinf;
  ASSERT_TRUE(std::isinf(inf));
  ASSERT_FALSE(std::isnan(inf));
  ASSERT_FALSE(std::isfinite(inf));

  fmc::decimal128 cninf = -std::numeric_limits<fmc::decimal128>::infinity();
  ASSERT_TRUE(std::isinf(cninf));
  ASSERT_FALSE(std::isnan(cninf));
  ASSERT_FALSE(std::isfinite(cninf));
  fmc_decimal128_t ninf = cninf;
  ASSERT_TRUE(std::isinf(ninf));
  ASSERT_FALSE(std::isnan(ninf));
  ASSERT_FALSE(std::isfinite(ninf));

  ASSERT_TRUE(cninf < cinf);
  ASSERT_TRUE(cninf <= cinf);
  ASSERT_FALSE(cinf < cninf);
  ASSERT_FALSE(cinf <= cninf);
  ASSERT_TRUE(ninf < inf);
  ASSERT_TRUE(ninf <= inf);
  ASSERT_FALSE(inf < ninf);
  ASSERT_FALSE(inf <= ninf);

  ASSERT_TRUE(cinf > cninf);
  ASSERT_TRUE(cinf >= cninf);
  ASSERT_FALSE(cninf > cinf);
  ASSERT_FALSE(cninf >= cinf);
  ASSERT_TRUE(inf > ninf);
  ASSERT_TRUE(inf >= ninf);
  ASSERT_FALSE(ninf > inf);
  ASSERT_FALSE(ninf >= inf);

  ASSERT_TRUE(ca < cinf);
  ASSERT_TRUE(ca <= cinf);
  ASSERT_FALSE(ca < cninf);
  ASSERT_FALSE(ca <= cninf);
  ASSERT_TRUE(a < inf);
  ASSERT_TRUE(a <= inf);
  ASSERT_FALSE(a < ninf);
  ASSERT_FALSE(a <= ninf);

  ASSERT_FALSE(ca > cinf);
  ASSERT_FALSE(ca >= cinf);
  ASSERT_TRUE(ca > cninf);
  ASSERT_TRUE(ca >= cninf);
  ASSERT_FALSE(a > inf);
  ASSERT_FALSE(a >= inf);
  ASSERT_TRUE(a > ninf);
  ASSERT_TRUE(a >= ninf);
}

TEST(decimal128, cppnan) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a(4);
  ASSERT_FALSE(fmc_decimal128_is_nan(&a));
  ASSERT_FALSE(fmc_decimal128_is_qnan(&a));
  ASSERT_FALSE(fmc_decimal128_is_snan(&a));
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));

  fmc::decimal128 qnan = std::numeric_limits<fmc::decimal128>::quiet_NaN();
  ASSERT_TRUE(fmc_decimal128_is_nan(&qnan));
  ASSERT_TRUE(fmc_decimal128_is_qnan(&qnan));
  ASSERT_FALSE(fmc_decimal128_is_snan(&qnan));
  ASSERT_FALSE(std::isinf(qnan));
  ASSERT_TRUE(std::isnan(qnan));

  fmc::decimal128 snan = std::numeric_limits<fmc::decimal128>::signaling_NaN();
  ASSERT_TRUE(fmc_decimal128_is_nan(&snan));
  ASSERT_FALSE(fmc_decimal128_is_qnan(&snan));
  ASSERT_TRUE(fmc_decimal128_is_snan(&snan));
  ASSERT_FALSE(std::isinf(snan));
  ASSERT_TRUE(std::isnan(snan));
}

TEST(decimal128, ostream) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a(5);
  a = a / (int64_t)10;
  std::ostringstream str;
  str << a;
  std::string res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "0.5");

  str.str("");
  str.clear();
  str << std::numeric_limits<fmc::decimal128>::infinity();
  res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "inf");

  str.str("");
  str.clear();
  str << std::numeric_limits<fmc::decimal128>::quiet_NaN();
  res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "nan");

  str.str("");
  str.clear();
  str << std::numeric_limits<fmc::decimal128>::signaling_NaN();
  res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "nan");

  str.str("");
  str.clear();
  fmc_decimal128_t c;
  fmc_decimal128_from_int(&c, 5);
  str << c;
  res = str.str();
  ASSERT_EQ(res.size(), 1);
  ASSERT_STREQ(res.c_str(), "5");
}

TEST(decimal128, cppmax) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a = std::numeric_limits<fmc::decimal128>::max();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, cppmin) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a = std::numeric_limits<fmc::decimal128>::min();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "-9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, cppepsilon) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a = std::numeric_limits<fmc::decimal128>::epsilon();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "0");
}

TEST(decimal128, cppdecimalfromint) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc::decimal128 a = 5;
  fmc::decimal128 b(5);
  int64_t ic = 5;
  fmc::decimal128 c(ic);
  ASSERT_EQ(a, b);
  ASSERT_EQ(a, c);
  fmc::decimal128 u = 5U;
  uint64_t ud = 5;
  fmc::decimal128 d(ud);
  ASSERT_EQ(a, u);
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "5");
}

TEST(decimal128, cppstreams) {
  feclearexcept(FE_ALL_EXCEPT);
  std::string s("432325555342");
  std::stringstream ss(s);
  fmc::decimal128 a((int64_t)432325555342);
  fmc::decimal128 b;
  ss >> b;
  ASSERT_EQ(a, b);
  fmc::decimal128 c((uint64_t)432325555342);
  ASSERT_EQ(c, b);
}

TEST(decimal128, invalid_cppstreams) {
  feclearexcept(FE_ALL_EXCEPT);
  std::string s("invaliddecimal");
  std::stringstream ss(s);
  fmc::decimal128 b;
  ASSERT_THROW(ss >> b, std::runtime_error);
}

void canonicalize(char *str, int keep_zeros) {
  for (auto i = strlen(str); --i > 0;) {
    if (str[i] == 'e') {
      str[i] = 'E';
      return;
    }
  }
  for (auto i = strlen(str); --i > 0;) {
    if (str[i] != '0') {
      for (auto j = i; j > 0; --j) {
        if (str[j] == '.') {
          if (i + keep_zeros == j) {
            str[j] = '\0';
          } else {
            str[i + keep_zeros + 1] = '\0';
          }
          break;
        }
      }
      break;
    }
  }
}

TEST(decimal128, identity_double_1) {
  feclearexcept(FE_ALL_EXCEPT);
  char float_str[256];
  char dec128_str[256];
  std::vector<double> decimals;
  std::vector<double> integers;
  std::vector<double> signs = {1.0, -1.0};
  for (double decimal = 0.0000019073486328125; decimal < 1.0; decimal *= 2.0) {
    decimals.push_back(decimal);
  }
  for (double integer = 1.0; integer <= 10779215329.0; integer *= 47.0) {
    integers.push_back(integer);
  }
  decimals.push_back(0.0);
  integers.push_back(0.0);
  fmc_error_t *err;
  for (auto &sign : signs) {
    for (auto &decimal : decimals) {
      for (auto &integer : integers) {
        for (int keep_zeros = 12; keep_zeros >= 0; keep_zeros -= 3) {
          double number = (decimal + integer) * sign;
          snprintf(float_str, sizeof(float_str) / sizeof(char), "%.33f",
                   number);
          feclearexcept(FE_ALL_EXCEPT); // Clear everything including sprintf
          canonicalize(float_str, keep_zeros);
          fmc_decimal128_t a;
          fmc_decimal128_from_str(&a, float_str, &err);
          ASSERT_EQ(err, nullptr);
          ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
          fmc_decimal128_to_str(dec128_str, &a);
          canonicalize(float_str, 0);
          EXPECT_STREQ(dec128_str, float_str);
          ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));

          fmc_decimal128_t b;
          fmc_decimal128_from_double(&b, number);
          ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
          fmc_decimal128_to_str(dec128_str, &b);
          ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
          EXPECT_STREQ(dec128_str, float_str);

          double converted;
          fmc_decimal128_to_double(&converted, &a);
          EXPECT_EQ(converted, number);
        }
      }
    }
  }
}

TEST(decimal128, identity_double_2) {
  double converted;
  double original;
  auto convert = [&](double number) {
    original = number;
    fmc_decimal128_t a;
    fmc_decimal128_from_double(&a, number);
    fmc_decimal128_to_double(&converted, &a);
  };

  convert(
      0.0000000000000000001074223844953689117151110868383255222897488847765205106053743033811542773037217557430267333984375);
  EXPECT_EQ(converted, original);
  convert(
      0.000000000000000000111517125575639286799006683430006308009755884298229238436872545747746698907576501369476318359375);
  EXPECT_EQ(converted, original);
  convert(
      -0.0000000000000000002718103111667332437926182560333733906119770008170846535555664758021521265618503093719482421875);
  EXPECT_EQ(converted, original);
  convert(-4412.3590295387484729872085154056549072265625);
  EXPECT_EQ(converted, original);
  convert(-5595.4715197982832251000218093395233154296875);
  EXPECT_EQ(converted, original);
  convert(14034.19794058726984076201915740966796875);
  EXPECT_EQ(converted, original);
  convert(24100.59487783108124858699738979339599609375);
  EXPECT_EQ(converted, original);
  convert(68379.09565273471525870263576507568359375);
  EXPECT_EQ(converted, original);
  convert(-75935.856705480997334234416484832763671875);
  EXPECT_EQ(converted, original);
  convert(86158.302008547514560632407665252685546875);
  EXPECT_EQ(converted, original);
  convert(598370.85907518700696527957916259765625);
  EXPECT_EQ(converted, original);
  convert(2360501.5847463184036314487457275390625);
  EXPECT_EQ(converted, original);
  convert(3920244.8222818146459758281707763671875);
  EXPECT_EQ(converted, original);
  convert(6030139.461389503441751003265380859375);
  EXPECT_EQ(converted, original);
  convert(9986334.22264326550066471099853515625);
  EXPECT_EQ(converted, original);
  convert(110367338.88334478437900543212890625);
  EXPECT_EQ(converted, original);
  convert(232863465.5218646228313446044921875);
  EXPECT_EQ(converted, original);
  convert(297581596.830785691738128662109375);
  EXPECT_EQ(converted, original);
  convert(364181915.17104339599609375);
  EXPECT_EQ(converted, original);
  convert(-445464211.388862311840057373046875);
  EXPECT_EQ(converted, original);
  convert(505788598.10503494739532470703125);
  EXPECT_EQ(converted, original);
  convert(538651182.4585511684417724609375);
  EXPECT_EQ(converted, original);
  convert(762722659.79106426239013671875);
  EXPECT_EQ(converted, original);
  convert(962612880.8451635837554931640625);
  EXPECT_EQ(converted, original);
  convert(976451324.891252994537353515625);
  EXPECT_EQ(converted, original);
  convert(-2277953182515.47265625);
  EXPECT_EQ(converted, original);
  convert(2696170217385.513671875);
  EXPECT_EQ(converted, original);
  convert(2926970179449.04052734375);
  EXPECT_EQ(converted, original);
  convert(3969252460837.57568359375);
  EXPECT_EQ(converted, original);
  convert(5888875542576.8427734375);
  EXPECT_EQ(converted, original);
  convert(6676174390470.93359375);
  EXPECT_EQ(converted, original);
  convert(6872537948155.7353515625);
  EXPECT_EQ(converted, original);
  convert(-7422573333287.1533203125);
  EXPECT_EQ(converted, original);
  convert(8030198688345.638671875);
  EXPECT_EQ(converted, original);
  convert(9901494043192.171875);
  EXPECT_EQ(converted, original);
  convert(82889918991823536.0);
  EXPECT_EQ(converted, original);
  convert(124536052943002528.0);
  EXPECT_EQ(converted, original);
  convert(144156884135999968.0);
  EXPECT_EQ(converted, original);
  convert(212839053581483936.0);
  EXPECT_EQ(converted, original);
  convert(218801490682184032.0);
  EXPECT_EQ(converted, original);
  convert(-536470472374601792.0);
  EXPECT_EQ(converted, original);
  convert(537654832563489984.0);
  EXPECT_EQ(converted, original);
  convert(652938785621731200.0);
  EXPECT_EQ(converted, original);
  convert(-659372859226975744.0);
  EXPECT_EQ(converted, original);
  convert(793418214483531648.0);
  EXPECT_EQ(converted, original);
}

TEST(decimal128, identity_extreme) {
  feclearexcept(FE_ALL_EXCEPT);
  char number_str[512];
  char dec_fromstr[512];
  char dec_fromdouble[512];
  double converted;

  auto to_str = [&](double number, int expected_signal = 0) {
    fmc_decimal128_t a;
    fmc_decimal128_t b;

#ifdef FMC_SYS_MACH
    auto isnan = std::isnan(number);
    auto isnegative = std::signbit(number);
    snprintf(number_str, sizeof(number_str) / sizeof(char), "%s%.34g",
             (isnan && isnegative) ? "-" : "", number);
#else
    snprintf(number_str, sizeof(number_str) / sizeof(char), "%.34g", number);
#endif

    feclearexcept(FE_ALL_EXCEPT); // Clear everything including sprintf

    fmc_error_t *err;
    ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
    fmc_decimal128_from_str(&a, number_str, &err);
    ASSERT_EQ(err, nullptr);
    ASSERT_FALSE(fetestexcept(FE_ALL_EXCEPT));
    fmc_decimal128_from_double(&b, number);
    ASSERT_EQ(fetestexcept(FE_ALL_EXCEPT), expected_signal);
    fmc_decimal128_to_str(dec_fromstr, &a);
    ASSERT_EQ(fetestexcept(FE_ALL_EXCEPT), expected_signal);
    fmc_decimal128_to_str(dec_fromdouble, &b);
    ASSERT_EQ(fetestexcept(FE_ALL_EXCEPT), expected_signal);

    canonicalize(number_str, 0);

    fmc_decimal128_to_double(&converted, &a);
  };

  to_str(std::numeric_limits<double>::quiet_NaN());
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_TRUE(std::isnan(converted));

  to_str(-std::numeric_limits<double>::quiet_NaN());
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_TRUE(std::isnan(converted));

  to_str(std::numeric_limits<double>::signaling_NaN());
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_TRUE(std::isnan(converted));

  to_str(-std::numeric_limits<double>::signaling_NaN());
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_TRUE(std::isnan(converted));

  to_str(std::numeric_limits<double>::infinity());
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_TRUE(std::isinf(converted));
  EXPECT_EQ(converted, std::numeric_limits<double>::infinity());

  to_str(-std::numeric_limits<double>::infinity());
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_TRUE(std::isinf(converted));
  EXPECT_EQ(converted, -std::numeric_limits<double>::infinity());

  to_str(std::numeric_limits<double>::denorm_min(), FE_INEXACT);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, std::numeric_limits<double>::denorm_min());

  to_str(-std::numeric_limits<double>::denorm_min(), FE_INEXACT);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, -std::numeric_limits<double>::denorm_min());

  to_str(std::numeric_limits<double>::denorm_min() * (double)(1ll << 55ll),
         FE_INEXACT);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted,
            std::numeric_limits<double>::denorm_min() * (double)(1ll << 55ll));

  to_str(std::numeric_limits<double>::max() / 128.0, FE_INEXACT);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, std::numeric_limits<double>::max() / 128.0);

  to_str(std::numeric_limits<double>::min() / 128.0, FE_INEXACT);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, std::numeric_limits<double>::min() / 128.0);

  to_str(0.0);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, 0.0);

  to_str(-0.0);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, -0.0);

  to_str(5.0);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, 5.0);

  to_str(-5.0);
  EXPECT_STREQ(dec_fromstr, number_str);
  EXPECT_STREQ(dec_fromdouble, number_str);
  EXPECT_EQ(converted, -5.0);
}

TEST(decimal128, exp63_check) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t table[18];
  fmc_error_t *err;
  fmc_decimal128_from_str(&table[0], "1", &err);
  fmc_decimal128_from_str(&table[1], "9223372036854775808", &err);
  fmc_decimal128_from_str(&table[2], "85070591730234615865843651857942052864",
                          &err);
  fmc_decimal128_from_str(
      &table[3], "784637716923335095479473677900958302012794430558004314112",
      &err);
  fmc_decimal128_from_str(
      &table[4],
      "72370055773322622139731865630429942408293740416025352524"
      "66099000494570602496",
      &err);
  fmc_decimal128_from_str(
      &table[5],
      "66749594872528440074844428317798503581334516323645399060"
      "845050244444366430645017188217565216768",
      &err);
  fmc_decimal128_from_str(
      &table[6],
      "6156563468186637376918600015647439657043709261010226041866920844413394"
      "02679643915803347910232576806887603562348544",
      &err);
  fmc_decimal128_from_str(
      &table[7],
      "5678427533559428832416592249125035424637823130369672345949142181098744"
      "438385921275985867583701277855943457200048954515105739075223552",
      &err);
  fmc_decimal128_from_str(
      &table[8],
      "52374249726338269920211035149241586435466272736689036631"
      "73266188953814074247479287813232147721446651441418694604"
      "0961136147476104734166288853256441430016",
      &err);
  fmc_decimal128_from_str(
      &table[9],
      "4830671903771572930869189863664984180373659162133043748321544064314398"
      "9278619505306702422082274032224530795200393777214717063483263037345696"
      "7863584183385093587122601852928",
      &err);
  fmc_decimal128_from_str(
      &table[10],
      "4455508415646675018204269146191690746966043464109921807206242693261010"
      "9054772240102596804798021205075963303804429632883893444382044682011701"
      "68614570041224793214838549179946240315306828365824",
      &err);
  fmc_decimal128_from_str(
      &table[11],
      "4109481173084666802532023346000100519961202970955604577733031955522446"
      "9955445943922763019814668659775210804444188892325882964314454560967680"
      "686052895717819140275184930690973423372373108471271228681978529185792",
      &err);
  fmc_decimal128_from_str(
      &table[12],
      "3790327373781027673703563204254156629045131877726310085788701264712038"
      "4587069748201437461153043126903088079362722926591947548340920771835728"
      "6202948008100864063587640630090308972232735749901964068667724412528434"
      "753635948938919936",
      &err);
  fmc_decimal128_from_str(
      &table[13],
      "3495959950985713037648777335653666807949431051290362857597833215614143"
      "5553409306835138286457305454559850292369652099267668941480416349336792"
      "5354321796442622320713797704824366482749038836413315139709961037985171"
      "4779776678907072458937421726941708288",
      &err);
  fmc_decimal128_from_str(
      &table[14],
      "3224453925388581825880980132547098428459761511450937024706791436930382"
      "7060346976286280350090799388909587060241287666545341940158661052584060"
      "7018419472009019109122731932986501567829295456803247713027485905890617"
      "92245363054977512313161523248215761503691988438775496704",
      &err);
  fmc_decimal128_from_str(
      &table[15],
      "2974033816955566125596124996299801120262520403318788918111543718631881"
      "3143208087470903366289923127011795974475803859461009091704910898114155"
      "8166116220478925156594168089491974788537281966859547374047839156470287"
      "4412135497413755760176314197880697316166024090210908287825647530697629"
      "36832",
      &err);
  fmc_decimal128_from_str(
      &table[16],
      "2743062034396844341627968125593604635037196317966166035056000994228098"
      "6908798364735825878497681813968066423626689360558724790919313723239516"
      "1205185912283514980724935035500313226779509889596701232075627063117989"
      "7595796976964454084495146379250195728106130226298287754794921070036903"
      "071843030324651025760256",
      &err);
  fmc_decimal128_from_str(
      &table[17],
      "2530028166341382729406191833986466338119458122051776479461266975342879"
      "2445999418361495047962679640561898384733039601488923726092173224184608"
      "3766749925923137401896780345707951705583634677616520426549709598090931"
      "3357025093542808658732726291945614494454260125706404484619404167682690"
      "3812816523290938580750782913463467636686848",
      &err);

  for (auto i = 0; i < 18; ++i) {
    EXPECT_EQ(fmc_decimal128_exp63[i], table[i]);
  }
}

TEST(decimal128, exp63_checkdec) {
  feclearexcept(FE_ALL_EXCEPT);
  fmc_decimal128_t table[18];
  fmc_error_t *err;
  fmc_decimal128_from_str(&table[0], "1", &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[1], "9223372036854775808", &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[2], "8507059173023461586584365185794205E+4",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[3], "7846377169233350954794736779009583E+23",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[4], "7237005577332262213973186563042994E+42",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[5], "6674959487252844007484442831779850E+61",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[6], "615656346818663737691860001564744E+81",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[7], "5678427533559428832416592249125035E+99",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[8], "5237424972633826992021103514924159E+118",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[9], "4830671903771572930869189863664984E+137",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[10], "4455508415646675018204269146191691E+156",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[11], "4109481173084666802532023346000101E+175",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[12], "3790327373781027673703563204254157E+194",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[13], "3495959950985713037648777335653667E+213",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[14], "3224453925388581825880980132547098E+232",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[15], "2974033816955566125596124996299801E+251",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[16], "2743062034396844341627968125593605E+270",
                          &err);
  EXPECT_EQ(err, nullptr);
  fmc_decimal128_from_str(&table[17], "2530028166341382729406191833986466E+289",
                          &err);
  EXPECT_EQ(err, nullptr);

  for (auto i = 0; i < 18; ++i) {
    EXPECT_EQ(fmc_decimal128_exp63[i], table[i]);
  }
}

TEST(decimal128, log10) {
  int actual;
  int result;
  auto t = [&](double n) {
    fmc_decimal128_t a;
    fmc_decimal128_from_double(&a, n);
    actual = (std::isfinite(n) && n != 0.0) ? floor(log10(abs(n))) : INT32_MIN;
    result = fmc_decimal128_flog10abs(&a);
  };

  t(-2.0);
  EXPECT_EQ(result, actual);
  t(0.0);
  EXPECT_EQ(result, actual);
  t(0.00048828125);
  EXPECT_EQ(result, actual);
  t(0.0625);
  EXPECT_EQ(result, actual);
  t(0.5);
  EXPECT_EQ(result, actual);
  t(2.0);
  EXPECT_EQ(result, actual);
  t(101.0);
  EXPECT_EQ(result, actual);
  t(744.015625);
  EXPECT_EQ(result, actual);
  t(1085.0);
  EXPECT_EQ(result, actual);
  t(4631.0);
  EXPECT_EQ(result, actual);
  t(9111.25);
  EXPECT_EQ(result, actual);
  t(9999.0);
  EXPECT_EQ(result, actual);
  t(10001.0);
  EXPECT_EQ(result, actual);
  t(64541.00048828125);
  EXPECT_EQ(result, actual);
  t(99781.0);
  EXPECT_EQ(result, actual);
  t(105011.0);
  EXPECT_EQ(result, actual);
  t(670964.1);
  EXPECT_EQ(result, actual);
  t(999999.99999);
  EXPECT_EQ(result, actual);
  t(1000000.25);
  EXPECT_EQ(result, actual);
  t(4177892.0);
  EXPECT_EQ(result, actual);
  t(std::numeric_limits<double>::max());
  EXPECT_EQ(result, actual);
  t(std::numeric_limits<double>::min());
  EXPECT_EQ(result, actual);
  t(std::numeric_limits<double>::denorm_min());
  EXPECT_EQ(result, actual);
  t(std::numeric_limits<double>::infinity());
  EXPECT_EQ(result, actual);
  t(-std::numeric_limits<double>::infinity());
  EXPECT_EQ(result, actual);
  t(std::numeric_limits<double>::quiet_NaN());
  EXPECT_EQ(result, actual);
  t(-std::numeric_limits<double>::quiet_NaN());
  EXPECT_EQ(result, actual);
}

TEST(decimal128, assign) {
  fmc::decimal128 a = 5;
  fmc::decimal128 b = a;
  ASSERT_EQ(a, b);
}

TEST(decimal128, move) {
  fmc::decimal128 a = 5;
  fmc::decimal128 b = std::move(a);
  fmc::decimal128 c = 5;
  ASSERT_EQ(b, c);
}

TEST(decimal128, cannonicalize) {
  fmc::decimal128 a(5);
  fmc::decimal128 cannon;
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "9999999999999999999999999999999999", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "-9999999999999999999999999999999999", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "9999999999999999999999999999999999E32", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "-9999999999999999999999999999999999E32", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  a = 0;
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 34);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "-0.0", &err);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  a = 22;
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  a = -22;
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "9099999999999999999999999999999999", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "-9099999999999999999999999999999999", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "9099999999999999999999999999999999E55", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "-9099999999999999999999999999999999E55", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  a = 222;
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  a = -222;
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "999999999999999999999999999999999", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "999999999999999999999999999999999E55", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(fmc_decimal128_lead_zeros(&cannon), 0);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "inf", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "-inf", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(a, cannon);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));

  fmc_decimal128_from_str(&a, "NaN", &err);
  fmc_decimal128_stdrep(&cannon, &a);
  EXPECT_EQ(std::hash<fmc_decimal128_t>{}(a),
            std::hash<fmc_decimal128_t>{}(cannon));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
