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
#include <fmc++/gtestwrap.hpp>
#include <random>
#include <string.h>

// C API

TEST(decimal128, from_to_flt_str) {
  fmc_decimal128_t a;
  std::string strval = "11111211111.111111111114111111";
  fmc_decimal128_from_str(&a, strval.c_str());
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, strval.c_str());
}

TEST(decimal128, from_uint_zero) {
  uint64_t max = 0;
  fmc_decimal128_t a = fmc_decimal128_from_uint(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "0");
}

TEST(decimal128, from_uint_low) {
  uint64_t max = 15;
  fmc_decimal128_t a = fmc_decimal128_from_uint(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "15");
}

TEST(decimal128, from_uint_extreme) {
  uint64_t max = std::numeric_limits<uint64_t>::max();
  fmc_decimal128_t a = fmc_decimal128_from_uint(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "18446744073709551615");
}

TEST(decimal128, from_int_zero) {
  int64_t max = 0;
  fmc_decimal128_t a = fmc_decimal128_from_int(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "0");
}

TEST(decimal128, from_int_low) {
  int64_t max = 15;
  fmc_decimal128_t a = fmc_decimal128_from_int(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "15");
}

TEST(decimal128, from_int_extreme) {
  int64_t max = std::numeric_limits<int64_t>::max();
  fmc_decimal128_t a = fmc_decimal128_from_int(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "9223372036854775807");
}

TEST(decimal128, from_int_neg_low) {
  int64_t max = -15;
  fmc_decimal128_t a = fmc_decimal128_from_int(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "-15");
}

TEST(decimal128, from_int_neg_extreme) {
  int64_t max = std::numeric_limits<int64_t>::min();
  fmc_decimal128_t a = fmc_decimal128_from_int(max);
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "-9223372036854775808");
}

TEST(decimal128, from_to_int_str) {
  fmc_decimal128_t a;
  std::string strval = "5";
  fmc_decimal128_from_str(&a, strval.c_str());
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, strval.c_str());
}

TEST(decimal128, divide) {
  fmc_decimal128_t a, b;
  char str[256];
  fmc_decimal128_from_str(&a, "66666666666.66666666666666666");
  fmc_decimal128_from_str(&b, "2.2");
  a = fmc_decimal128_div(a, b);
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "30303030303.0303030303030303");
}

TEST(decimal128, int_div) {
  fmc_decimal128_t a;
  char str[256];
  fmc_decimal128_from_str(&a, "66666666666.66666666666666666");
  a = fmc_decimal128_int_div(a, 2);
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "33333333333.33333333333333333");
}

TEST(decimal128, add) {
  fmc_decimal128_t a, b;
  char str[256];
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111");
  fmc_decimal128_from_str(&b, "22222222222.22222222222222222");
  a = fmc_decimal128_add(a, b);
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "33333333333.33333333333333333");
}

TEST(decimal128, sub) {
  fmc_decimal128_t a, b;
  char str[256];
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111");
  fmc_decimal128_from_str(&b, "22222222222.22222222222222222");
  a = fmc_decimal128_sub(a, b);
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "-11111111111.11111111111111111");
}

TEST(decimal128, mul) {
  fmc_decimal128_t a, b;
  char str[256];
  fmc_decimal128_from_str(&a, "11111111111.11111111111111111");
  fmc_decimal128_from_str(&b, "2.2");
  a = fmc_decimal128_mul(a, b);
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "24444444444.444444444444444442");
}

TEST(decimal128, comparison) {
  fmc_decimal128_t a, b, c;
  fmc_decimal128_from_str(&a, "2.0");
  fmc_decimal128_from_str(&b, "2.5");
  fmc_decimal128_from_str(&c, "3.0");

  ASSERT_FALSE(fmc_decimal128_less(b, a));
  ASSERT_FALSE(fmc_decimal128_less(b, b));
  ASSERT_TRUE(fmc_decimal128_less(b, c));

  ASSERT_FALSE(fmc_decimal128_less_or_equal(b, a));
  ASSERT_TRUE(fmc_decimal128_less_or_equal(b, b));
  ASSERT_TRUE(fmc_decimal128_less_or_equal(b, c));

  ASSERT_TRUE(fmc_decimal128_greater(b, a));
  ASSERT_FALSE(fmc_decimal128_greater(b, b));
  ASSERT_FALSE(fmc_decimal128_greater(b, c));

  ASSERT_TRUE(fmc_decimal128_greater_or_equal(b, a));
  ASSERT_TRUE(fmc_decimal128_greater_or_equal(b, b));
  ASSERT_FALSE(fmc_decimal128_greater_or_equal(b, c));

  ASSERT_FALSE(fmc_decimal128_equal(b, a));
  ASSERT_TRUE(fmc_decimal128_equal(b, b));
  ASSERT_FALSE(fmc_decimal128_equal(b, c));
}

TEST(decimal128, round) {
  fmc_decimal128_t a, b, c, d, e;
  fmc_decimal128_from_str(&a, "2.0");
  fmc_decimal128_from_str(&b, "2.2");
  fmc_decimal128_from_str(&c, "2.5");
  fmc_decimal128_from_str(&d, "2.7");
  fmc_decimal128_from_str(&e, "3.0");
  fmc_decimal128_t ar, br, cr, dr;
  ar = fmc_decimal128_round(a);
  br = fmc_decimal128_round(b);
  cr = fmc_decimal128_round(c);
  dr = fmc_decimal128_round(d);
  ASSERT_TRUE(fmc_decimal128_equal(a, ar));
  ASSERT_TRUE(fmc_decimal128_equal(a, br));
  ASSERT_TRUE(fmc_decimal128_equal(e, cr));
  ASSERT_TRUE(fmc_decimal128_equal(e, dr));
}

TEST(decimal128, increment) {
  fmc_decimal128_t a, b;
  fmc_decimal128_from_str(&a, "2.0");
  fmc_decimal128_from_str(&b, "4.0");
  fmc_decimal128_inc(&a, a);
  ASSERT_TRUE(fmc_decimal128_equal(a, b));
}

TEST(decimal128, decrement) {
  fmc_decimal128_t a, b;
  fmc_decimal128_from_str(&a, "4.0");
  fmc_decimal128_from_str(&b, "2.0");
  fmc_decimal128_dec(&a, b);
  ASSERT_TRUE(fmc_decimal128_equal(a, b));
}

TEST(decimal128, negate) {
  fmc_decimal128_t a;
  fmc_decimal128_from_str(&a, "4.0");

  fmc_decimal128_t b = fmc_decimal128_negate(a);
  ASSERT_FALSE(fmc_decimal128_equal(a, b));

  b = fmc_decimal128_negate(b);
  ASSERT_TRUE(fmc_decimal128_equal(a, b));
}

TEST(decimal128, infinity) {
  fmc_decimal128_t a;
  fmc_decimal128_from_str(&a, "4.0");
  ASSERT_FALSE(fmc_decimal128_is_inf(a));

  fmc_decimal128_t inf = fmc_decimal128_inf();
  ASSERT_TRUE(fmc_decimal128_is_inf(inf));
}

TEST(decimal128, nan) {
  fmc_decimal128_t a;
  fmc_decimal128_from_str(&a, "4.0");
  ASSERT_FALSE(fmc_decimal128_is_nan(a));
  ASSERT_FALSE(fmc_decimal128_is_qnan(a));
  ASSERT_FALSE(fmc_decimal128_is_snan(a));

  fmc_decimal128_t qnan = fmc_decimal128_qnan();
  ASSERT_TRUE(fmc_decimal128_is_nan(qnan));
  ASSERT_TRUE(fmc_decimal128_is_qnan(qnan));
  ASSERT_FALSE(fmc_decimal128_is_snan(qnan));
  ASSERT_FALSE(fmc_decimal128_is_inf(qnan));

  fmc_decimal128_t snan = fmc_decimal128_snan();
  ASSERT_TRUE(fmc_decimal128_is_nan(snan));
  ASSERT_FALSE(fmc_decimal128_is_qnan(snan));
  ASSERT_TRUE(fmc_decimal128_is_snan(snan));
  ASSERT_FALSE(fmc_decimal128_is_inf(snan));
}

TEST(decimal128, max) {
  fmc_decimal128_t a = fmc_decimal128_max();
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "9.999999999999999999999999999999999E+6144");
}

TEST(decimal128, min) {
  fmc_decimal128_t a = fmc_decimal128_min();
  char str[256];
  fmc_decimal128_to_str(a, str);
  ASSERT_STREQ(str, "-9.999999999999999999999999999999999E+6144");
}

// C++ API
TEST(decimal128, cppconstructor) {
  fmc_decimal128_t a;
  fmc_decimal128_from_str(&a, "5");

  fmc::decimal128 ppa(5.0);
  fmc::decimal128 ppb(a);
  ASSERT_TRUE(fmc_decimal128_equal(a, ppa));
  ASSERT_TRUE(fmc_decimal128_equal(a, ppb));
}

TEST(decimal128, cppdivide) {
  fmc::decimal128 cppa(10.0);
  fmc::decimal128 cppb(5.0);
  fmc::decimal128 cppc(2.0);
  ASSERT_EQ(cppa / cppb, cppc);

  fmc_decimal128_t ppa = fmc_decimal128_from_double(10.0);
  fmc_decimal128_t ppb = fmc_decimal128_from_double(5.0);
  fmc_decimal128_t ppc = fmc_decimal128_from_double(2.0);
  ASSERT_EQ(ppa / ppb, ppc);
}

TEST(decimal128, cppint_div) {
  fmc::decimal128 cppa(10.0);
  fmc::decimal128 cppb(2.0);
  ASSERT_EQ(cppa / (int64_t)5, cppb);

  fmc_decimal128_t ppa = fmc_decimal128_from_double(10.0);
  fmc_decimal128_t ppb = fmc_decimal128_from_double(2.0);
  ASSERT_EQ(ppa / (int64_t)5, ppb);
}

TEST(decimal128, cppadd) {
  fmc::decimal128 cppa(7.0);
  fmc::decimal128 cppb(5.0);
  fmc::decimal128 cppc(2.0);
  ASSERT_EQ(cppc + cppb, cppa);

  fmc_decimal128_t ppa = fmc_decimal128_from_double(7.0);
  fmc_decimal128_t ppb = fmc_decimal128_from_double(5.0);
  fmc_decimal128_t ppc = fmc_decimal128_from_double(2.0);
  ASSERT_EQ(ppc + ppb, ppa);
}

TEST(decimal128, cppsub) {
  fmc::decimal128 cppa(7.0);
  fmc::decimal128 cppb(5.0);
  fmc::decimal128 cppc(2.0);
  ASSERT_EQ(cppa - cppb, cppc);

  fmc_decimal128_t ppa = fmc_decimal128_from_double(7.0);
  fmc_decimal128_t ppb = fmc_decimal128_from_double(5.0);
  fmc_decimal128_t ppc = fmc_decimal128_from_double(2.0);
  ASSERT_EQ(ppa - ppb, ppc);
}

TEST(decimal128, cppmul) {
  fmc::decimal128 cppa(10.0);
  fmc::decimal128 cppb(5.0);
  fmc::decimal128 cppc(2.0);
  ASSERT_EQ(cppc * cppb, cppa);

  fmc_decimal128_t ppa = fmc_decimal128_from_double(10.0);
  fmc_decimal128_t ppb = fmc_decimal128_from_double(5.0);
  fmc_decimal128_t ppc = fmc_decimal128_from_double(2.0);
  ASSERT_EQ(ppc * ppb, ppa);
}

TEST(decimal128, cppcomparison) {
  fmc::decimal128 cppa(5.0);
  fmc::decimal128 cppb(8.0);
  fmc::decimal128 cppc(10.0);

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

  fmc_decimal128_t ppa = fmc_decimal128_from_double(5.0);
  fmc_decimal128_t ppb = fmc_decimal128_from_double(8.0);
  fmc_decimal128_t ppc = fmc_decimal128_from_double(10.0);

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
  fmc::decimal128 cppa(5.0);
  fmc::decimal128 cppb(10.0);
  cppa += cppa;
  ASSERT_EQ(cppa, cppb);
}

TEST(decimal128, cppdecrement) {
  fmc::decimal128 cppa(10.0);
  fmc::decimal128 cppb(5.0);
  cppa -= cppb;
  ASSERT_EQ(cppa, cppb);
}

TEST(decimal128, cppupcasting) {
  fmc_decimal128_t a;
  fmc::decimal128 &ppa = fmc::decimal128::upcast(a);
  ASSERT_EQ(&a, &ppa);
}

TEST(decimal128, cppimplicit_downcasting) {
  fmc::decimal128 ppa(5.0);
  auto f = [](const fmc::decimal128 &lhs, const fmc_decimal128_t &rhs) -> bool {
    return &lhs == &rhs;
  };
  ASSERT_TRUE(f(ppa, ppa));
}

TEST(decimal128, cppnegate) {
  fmc::decimal128 a(4.0);
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));
  ASSERT_TRUE(std::isfinite(a));

  fmc::decimal128 b = -a;

  ASSERT_NE(a, b);
  ASSERT_EQ(a, -b);
}

TEST(decimal128, cppinfinity) {
  fmc::decimal128 ca(4.0);
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
  fmc::decimal128 a(4.0);
  ASSERT_FALSE(fmc_decimal128_is_nan(a));
  ASSERT_FALSE(fmc_decimal128_is_qnan(a));
  ASSERT_FALSE(fmc_decimal128_is_snan(a));
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));

  fmc::decimal128 qnan = std::numeric_limits<fmc::decimal128>::quiet_NaN();
  ASSERT_TRUE(fmc_decimal128_is_nan(qnan));
  ASSERT_TRUE(fmc_decimal128_is_qnan(qnan));
  ASSERT_FALSE(fmc_decimal128_is_snan(qnan));
  ASSERT_FALSE(std::isinf(qnan));
  ASSERT_TRUE(std::isnan(qnan));

  fmc::decimal128 snan = std::numeric_limits<fmc::decimal128>::signaling_NaN();
  ASSERT_TRUE(fmc_decimal128_is_nan(snan));
  ASSERT_FALSE(fmc_decimal128_is_qnan(snan));
  ASSERT_TRUE(fmc_decimal128_is_snan(snan));
  ASSERT_FALSE(std::isinf(snan));
  ASSERT_TRUE(std::isnan(snan));
}

TEST(decimal128, ostream) {
  fmc::decimal128 a(5.0);
  a = a / (int64_t)10;
  std::ostringstream str;
  str << a;
  std::string res = str.str();
  ASSERT_EQ(res.size(), 8);
  ASSERT_STREQ(res.c_str(), "0.500000");

  str.str("");
  str.clear();
  str << std::numeric_limits<fmc::decimal128>::infinity();
  res = str.str();
  ASSERT_EQ(res.size(), 8);
  ASSERT_STREQ(res.c_str(), "Infinity");

  str.str("");
  str.clear();
  str << std::numeric_limits<fmc::decimal128>::quiet_NaN();
  res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "NaN");

  str.str("");
  str.clear();
  str << std::numeric_limits<fmc::decimal128>::signaling_NaN();
  res = str.str();
  ASSERT_EQ(res.size(), 4);
  ASSERT_STREQ(res.c_str(), "sNaN");

  str.str("");
  str.clear();
  fmc_decimal128_t c = fmc_decimal128_from_double(0.5);
  str << c;
  res = str.str();
  ASSERT_EQ(res.size(), 8);
  ASSERT_STREQ(res.c_str(), "0.500000");
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

TEST(decimal128, cppdecimalfromdouble) {
  fmc::decimal128 a = 2.33;
  fmc::decimal128 b = 2.33f;
  fmc::decimal128 c(2.33);
  fmc::decimal128 d(2.33f);
  double de = 2.33;
  fmc::decimal128 e(de);
  float ff = 2.33;
  fmc::decimal128 f(ff);
  ASSERT_EQ(a, b);
  ASSERT_EQ(a, c);
  ASSERT_EQ(a, d);
  ASSERT_EQ(a, e);
  ASSERT_EQ(a, f);
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "2.330000");
  double implicitoutput = a;
  double explicitoutput = (double)a;
  ASSERT_DOUBLE_EQ(de, implicitoutput);
  ASSERT_DOUBLE_EQ(de, explicitoutput);
}

TEST(decimal128, cppdecimalfromint) {
  fmc::decimal128 a = (int64_t)5;
  fmc::decimal128 b((int64_t)5);
  int64_t ic = 5;
  fmc::decimal128 c(ic);
  ASSERT_EQ(a, b);
  ASSERT_EQ(a, c);
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "5");
}

TEST(decimal128, cppstreams) {
  std::string s("432325.555342");
  std::stringstream ss(s);
  fmc::decimal128 a(432325.555342);
  fmc::decimal128 b(0.0);
  ss >> b;
  ASSERT_EQ(a, b);
}

TEST(decimal128, rand) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> distrib(1, 1000000000);
  for (int i = 0; i < 1000; ++i) {
    int64_t r = distrib(gen);
    std::cout << "random value " << r << std::endl;
    r /= 1000000;

    // divide the random integer by 10^6
    // get fraction that potentially starts with zeros
    // print the original number, convert it to decimal, take the decimal and
    // write down string
  }
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
