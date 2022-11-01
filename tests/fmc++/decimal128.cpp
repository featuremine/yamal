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

#include "fmc/error.h"
#include "fmc/decimal128.h"
#include "fmc++/decimal128.hpp"
#include <fmc++/gtestwrap.hpp>
#include <random>
#include <string.h>

// C API

TEST(decimal128, from_to_flt_str) {
  fmc_decimal128_t a;
  std::string strval = "11111211111.111111111114111111";
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, strval.c_str());
}

TEST(decimal128, bad_str) {
  fmc_decimal128_t a;

  // Non numerical string
  std::string strval = "thisisnotanumber";
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);

  // Invalid numerical string
  strval = "1.2.3";
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);

  // Numerical string length
  strval = std::string(34, '1');  // supports up to 34 decimal digits
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  strval = std::string("-") + std::string(34, '1');
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  strval = std::string("1.") + std::string(33, '1') + std::string("E+6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  strval = std::string("1.") + std::string(33, '1') + std::string("E-6143");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  strval = std::string("-1.") + std::string(33, '1') + std::string("E+6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  strval = std::string("-1.") + std::string(33, '1') + std::string("E-6143");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);

  strval = std::string(35, '1');  // supports up to 34 decimal digits
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  strval = std::string("-") + std::string(35, '1');
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  strval = std::string("1.") + std::string(33, '1') + std::string("E+6145");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  strval = std::string("1.") + std::string(33, '1') + std::string("E-6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  strval = std::string("-1.") + std::string(33, '1') + std::string("E+6145");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
  strval = std::string("-1.") + std::string(33, '1') + std::string("E-6144");
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_NE(err, nullptr);
  fmc_error_clear(&err);
}

TEST(decimal128, from_uint_zero) {
  uint64_t max = 0;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_uint(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "0");
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_uint_low) {
  uint64_t max = 15;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_uint(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "15");
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_uint_extreme) {
  uint64_t max = std::numeric_limits<uint64_t>::max();
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_uint(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "18446744073709551615");
  uint64_t res;
  fmc_decimal128_to_uint(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_int_zero) {
  int64_t max = 0;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "0");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_int_low) {
  int64_t max = 15;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "15");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_int_extreme) {
  int64_t max = std::numeric_limits<int64_t>::max();
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "9223372036854775807");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_int_neg_low) {
  int64_t max = -15;
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "-15");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_int_neg_extreme) {
  int64_t max = std::numeric_limits<int64_t>::min();
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_int(&a, max);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "-9223372036854775808");
  int64_t res;
  fmc_decimal128_to_int(&res, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_EQ(res, max);
}

TEST(decimal128, from_to_int_str) {
  fmc_decimal128_t a;
  std::string strval = "5";
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, strval.c_str(), &err);
  ASSERT_EQ(err, nullptr);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, strval.c_str());
}

TEST(decimal128, divide) {
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "66666666666.66666666666666666", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "2.2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_div(&c, &a, &b, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "30303030303.0303030303030303");
}

TEST(decimal128, intdivide) {
  fmc_decimal128_t cppa;
  fmc_decimal128_from_int(&cppa, 10);
  int64_t b = 5;
  fmc_decimal128_t cppc;
  fmc_decimal128_from_int(&cppc, 2);
  ASSERT_EQ(cppa / b, cppc);
}

TEST(decimal128, add) {
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "22222222222.22222222222222222", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_add(&c, &a, &b, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "33333333333.33333333333333333");
}

TEST(decimal128, sub) {
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "22222222222.22222222222222222", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_sub(&c, &a, &b, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "-11111111111.11111111111111111");
}

TEST(decimal128, mul) {
  fmc_decimal128_t a, b, c;
  char str[256];
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "2.2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_mul(&c, &a, &b, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_to_str(str, &c);
  ASSERT_STREQ(str, "24444444444.444444444444444442");
}

TEST(decimal128, comparison) {
  fmc_decimal128_t a, b, c;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "2.5", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&c, "3", &err);
  ASSERT_EQ(err, nullptr);

  ASSERT_FALSE(fmc_decimal128_less(&b, &a, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fmc_decimal128_less(&b, &b, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_less(&b, &c, &err));
  ASSERT_EQ(err, nullptr);

  ASSERT_FALSE(fmc_decimal128_less_or_equal(&b, &a, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_less_or_equal(&b, &b, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_less_or_equal(&b, &c, &err));
  ASSERT_EQ(err, nullptr);

  ASSERT_TRUE(fmc_decimal128_greater(&b, &a, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fmc_decimal128_greater(&b, &b, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fmc_decimal128_greater(&b, &c, &err));
  ASSERT_EQ(err, nullptr);

  ASSERT_TRUE(fmc_decimal128_greater_or_equal(&b, &a, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_greater_or_equal(&b, &b, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fmc_decimal128_greater_or_equal(&b, &c, &err));

  ASSERT_FALSE(fmc_decimal128_equal(&b, &a, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&b, &b, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fmc_decimal128_equal(&b, &c, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, round) {
  fmc_decimal128_t a, b, c, d, e;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "2.2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&c, "2.5", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&d, "2.7", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&e, "3", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_t ar, br, cr, dr;
  fmc_decimal128_round(&ar, &a, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_round(&br, &b, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_round(&cr, &c, &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_round(&dr, &d, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &ar, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &br, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&e, &cr, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&e, &dr, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, increment) {
  fmc_decimal128_t a, b;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "4", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_inc(&a, &a, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &b, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, decrement) {
  fmc_decimal128_t a, b;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_from_str(&b, "2", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_dec(&a, &b, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &b, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, negate) {
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);

  fmc_decimal128_t b;
  fmc_decimal128_negate(&b, &a, &err);
  ASSERT_FALSE(fmc_decimal128_equal(&a, &b, &err));
  ASSERT_EQ(err, nullptr);

  fmc_decimal128_t c;
  fmc_decimal128_negate(&c, &b, &err);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &c, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, pow10) {
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);

  fmc_decimal128_pow10(&a, 3, &err);
  ASSERT_EQ(err, nullptr);

  fmc_decimal128_t b;
  fmc_decimal128_from_str(&b, "4000", &err);
  ASSERT_EQ(err, nullptr);

  ASSERT_TRUE(fmc_decimal128_equal(&a, &b, &err));
  ASSERT_EQ(err, nullptr);

  fmc_decimal128_t c;
  fmc_decimal128_from_str(&c, "40", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_pow10(&a, -2, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &c, &err));
  ASSERT_EQ(err, nullptr);

  fmc_decimal128_t d;
  fmc_decimal128_from_str(&d, "0.04", &err);
  ASSERT_EQ(err, nullptr);
  fmc_decimal128_pow10(&a, -3, &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &d, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, infinity) {
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
  ASSERT_FALSE(fmc_decimal128_is_inf(&a));

  fmc_decimal128_t inf;
  fmc_decimal128_inf(&inf);
  ASSERT_TRUE(fmc_decimal128_is_inf(&inf));
}

TEST(decimal128, nan) {
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "4", &err);
  ASSERT_EQ(err, nullptr);
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
  fmc_decimal128_t a;
  fmc_decimal128_max(&a);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, min) {
  fmc_decimal128_t a;
  fmc_decimal128_min(&a);
  char str[256];
  fmc_decimal128_to_str(str, &a);
  ASSERT_STREQ(str, "-9.999999999999999999999999999999999E+6144");
}

// C++ API
TEST(decimal128, cppconstructor) {
  fmc_decimal128_t a;
  fmc_error_t *err;
  fmc_decimal128_from_str(&a, "5", &err);
  ASSERT_EQ(err, nullptr);

  fmc::decimal128 ppa(5);
  fmc::decimal128 ppb(a);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &ppa, &err));
  ASSERT_EQ(err, nullptr);
  ASSERT_TRUE(fmc_decimal128_equal(&a, &ppb, &err));
  ASSERT_EQ(err, nullptr);

  fmc::decimal128 ppc;
  ppa = ppa - ppa;
  ASSERT_TRUE(fmc_decimal128_equal(&ppa, &ppc, &err));
  ASSERT_EQ(err, nullptr);
}

TEST(decimal128, cppdivide) {
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
  fmc::decimal128 cppa(10);
  int64_t b = 5;
  fmc::decimal128 cppc(2);
  ASSERT_EQ(cppa / b, cppc);
}

TEST(decimal128, cppadd) {
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
  fmc::decimal128 cppa(5);
  fmc::decimal128 cppb(10);
  cppa += cppa;
  ASSERT_EQ(cppa, cppb);
}

TEST(decimal128, cppdecrement) {
  fmc::decimal128 cppa(10);
  fmc::decimal128 cppb(5);
  cppa -= cppb;
  ASSERT_EQ(cppa, cppb);
}

TEST(decimal128, cppupcasting) {
  fmc_decimal128_t a;
  fmc::decimal128 &ppa = fmc::decimal128::upcast(a);
  ASSERT_EQ(&a, &ppa);
}

TEST(decimal128, cppimplicit_downcasting) {
  fmc::decimal128 ppa(5);
  auto f = [](const fmc::decimal128 &lhs, const fmc_decimal128_t &rhs) -> bool {
    return &lhs == &rhs;
  };
  ASSERT_TRUE(f(ppa, ppa));
}

TEST(decimal128, cppnegate) {
  fmc::decimal128 a(4);
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));
  ASSERT_TRUE(std::isfinite(a));

  fmc::decimal128 b = -a;

  ASSERT_NE(a, b);
  ASSERT_EQ(a, -b);
}

TEST(decimal128, cppinfinity) {
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
  fmc::decimal128 a = std::numeric_limits<fmc::decimal128>::max();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, cppmin) {
  fmc::decimal128 a = std::numeric_limits<fmc::decimal128>::min();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "-9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, cppepsilon) {
  fmc::decimal128 a = std::numeric_limits<fmc::decimal128>::epsilon();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "0");
}

TEST(decimal128, cppdecimalfromint) {
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
  std::string s("432325555342");
  std::stringstream ss(s);
  fmc::decimal128 a((int64_t)432325555342);
  fmc::decimal128 b;
  ss >> b;
  ASSERT_EQ(a, b);
  fmc::decimal128 c((uint64_t)432325555342);
  ASSERT_EQ(c, b);
}

void keep_some_zeros(char *str, int n) {
  for (auto i = strlen(str); --i > 0;) {
    if (str[i] != '0') {
      for (auto j = i; j > 0; --j) {
        if (str[j] == '.') {
          if (i + n == j) {
            str[j] = '\0';
          } else {
            str[i + n + 1] = '\0';
          }
        }
      }
      break;
    }
  }
}

TEST(decimal128, identity_double) {
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
          sprintf(float_str, "%.33f", number);
          keep_some_zeros(float_str, keep_zeros);
          fmc_decimal128_t a;
          fmc_decimal128_from_str(&a, float_str, &err);
          ASSERT_EQ(err, nullptr);
          fmc_decimal128_to_str(dec128_str, &a);
          keep_some_zeros(float_str, 0);
          EXPECT_STREQ(dec128_str, float_str);
        }
      }
    }
  }
}

TEST(decimal128, identity_infnan) {
  char float_str[256];
  char dec128_str[256];

  auto to_str = [&](double number) {
    fmc_decimal128_t a;
    fmc_error_t *err;
    sprintf(float_str, "%.33f", number);
    fmc_decimal128_from_str(&a, float_str, &err);
    ASSERT_EQ(err, nullptr);
    fmc_decimal128_to_str(dec128_str, &a);
  };

  to_str(std::numeric_limits<double>::quiet_NaN());
  EXPECT_STREQ(dec128_str, float_str);

  to_str(-std::numeric_limits<double>::quiet_NaN());
  EXPECT_STREQ(dec128_str, float_str);

  to_str(std::numeric_limits<double>::infinity());
  EXPECT_STREQ(dec128_str, float_str);

  to_str(-std::numeric_limits<double>::infinity());
  EXPECT_STREQ(dec128_str, float_str);
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

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
