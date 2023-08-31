/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.

        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file rprice.cpp
 * @date 19 Oct 2022
 * @brief File contains tests for rprice
 *
 * @see http://www.featuremine.com
 */

#include "fmc/rprice.h"
#include "fmc++/rprice.hpp"
#include "fmc/error.h"
#include <fmc++/gtestwrap.hpp>
#include <libdecnumber/decQuad.h>
#include <random>
#include <string.h>

// C API
//   9223372036

TEST(rprice, from_int_zero) {
  int64_t max = 0;
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, max);
  int64_t res;
  fmc_rprice_to_int(&res, &a);
  ASSERT_EQ(res, max);
}

TEST(rprice, from_int_low) {
  int64_t max = 15;
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, max);
  int64_t res;
  fmc_rprice_to_int(&res, &a);
  ASSERT_EQ(res, max);
}

TEST(rprice, from_int_extreme) {
  int64_t max = std::numeric_limits<int64_t>::max() / FMC_RPRICE_FRACTION;
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, max);
  int64_t res;
  fmc_rprice_to_int(&res, &a);
  ASSERT_EQ(res, max);
}

TEST(rprice, from_int_neg_low) {
  int64_t max = -15;
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, max);
  int64_t res;
  fmc_rprice_to_int(&res, &a);
  ASSERT_EQ(res, max);
}

TEST(rprice, from_int_neg_extreme) {
  int64_t max = std::numeric_limits<int64_t>::min() / FMC_RPRICE_FRACTION;
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, max);
  int64_t res;
  fmc_rprice_to_int(&res, &a);
  ASSERT_EQ(res, max);
}

TEST(rprice, divide) {
  fmc_rprice_t a, b, c;
  double bc;
  fmc_rprice_from_double(&a, 666666666.666666);
  fmc_rprice_from_double(&b, 2.2);
  fmc_rprice_div(&c, &a, &b);
  fmc_rprice_to_double(&bc, &c);
  ASSERT_DOUBLE_EQ(bc, 303030303.0303027);
}

TEST(rprice, intdivide) {
  fmc_rprice_t cppa;
  fmc_rprice_from_int(&cppa, 10);
  int64_t b = 5;
  fmc_rprice_t cppc;
  fmc_rprice_from_int(&cppc, 2);
  ASSERT_EQ(cppa / b, cppc);
}

TEST(rprice, add) {
  fmc_rprice_t a, b, c;
  double bc;
  fmc_rprice_from_double(&a, 111111111.111111);
  fmc_rprice_from_double(&b, 222222222.222222);
  fmc_rprice_add(&c, &a, &b);
  fmc_rprice_to_double(&bc, &c);
  ASSERT_DOUBLE_EQ(bc, 333333333.333333);
}

TEST(rprice, sub) {
  fmc_rprice_t a, b, c;
  double bc;
  fmc_rprice_from_double(&a, 111111111.111111);
  fmc_rprice_from_double(&b, 222222222.222222);
  fmc_rprice_sub(&c, &a, &b);
  fmc_rprice_to_double(&bc, &c);
  ASSERT_DOUBLE_EQ(bc, -111111111.11111102);
}

TEST(rprice, mul) {
  fmc_rprice_t a, b, c;
  double bc;
  fmc_rprice_from_double(&a, 111111111.111111);
  fmc_rprice_from_double(&b, 2.2);
  fmc_rprice_mul(&c, &a, &b);
  fmc_rprice_to_double(&bc, &c);
  ASSERT_DOUBLE_EQ(bc, 244444444.4444442);
}

TEST(rprice, comparison) {
  fmc_rprice_t a, b, c;
  fmc_rprice_from_double(&a, 2.0);
  fmc_rprice_from_double(&b, 2.5);
  fmc_rprice_from_double(&c, 3);

  ASSERT_FALSE(fmc_rprice_less(&b, &a));
  ASSERT_FALSE(fmc_rprice_less(&b, &b));
  ASSERT_TRUE(fmc_rprice_less(&b, &c));

  ASSERT_FALSE(fmc_rprice_less_or_equal(&b, &a));
  ASSERT_TRUE(fmc_rprice_less_or_equal(&b, &b));
  ASSERT_TRUE(fmc_rprice_less_or_equal(&b, &c));

  ASSERT_TRUE(fmc_rprice_greater(&b, &a));
  ASSERT_FALSE(fmc_rprice_greater(&b, &b));
  ASSERT_FALSE(fmc_rprice_greater(&b, &c));

  ASSERT_TRUE(fmc_rprice_greater_or_equal(&b, &a));
  ASSERT_TRUE(fmc_rprice_greater_or_equal(&b, &b));
  ASSERT_FALSE(fmc_rprice_greater_or_equal(&b, &c));

  ASSERT_FALSE(fmc_rprice_equal(&b, &a));
  ASSERT_TRUE(fmc_rprice_equal(&b, &b));
  ASSERT_FALSE(fmc_rprice_equal(&b, &c));
}

TEST(rprice, increment) {
  fmc_rprice_t a, b;
  fmc_rprice_from_int(&a, 2);
  fmc_rprice_from_int(&b, 4);
  fmc_rprice_inc(&a, &a);
  ASSERT_TRUE(fmc_rprice_equal(&a, &b));
}

TEST(rprice, decrement) {
  fmc_rprice_t a, b;
  fmc_rprice_from_int(&a, 4);
  fmc_rprice_from_int(&b, 2);
  fmc_rprice_dec(&a, &b);
  ASSERT_TRUE(fmc_rprice_equal(&a, &b));
}

TEST(rprice, negate) {
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, 4);

  fmc_rprice_t b;
  fmc_rprice_negate(&b, &a);
  ASSERT_FALSE(fmc_rprice_equal(&a, &b));

  fmc_rprice_t c;
  fmc_rprice_negate(&c, &b);
  ASSERT_TRUE(fmc_rprice_equal(&a, &c));
}

TEST(rprice, max) {
  fmc_rprice_t a;
  fmc_rprice_max(&a);
  double da;
  fmc_rprice_to_double(&da, &a);
  ASSERT_STREQ(std::to_string(da).c_str(), "9223372036.854776");
}

TEST(rprice, min) {
  fmc_rprice_t a;
  fmc_rprice_min(&a);
  double da;
  fmc_rprice_to_double(&da, &a);
  ASSERT_STREQ(std::to_string(da).c_str(), "-9223372036.854776");
}

// C++ API
TEST(rprice, cppconstructor) {
  fmc_rprice_t a;
  fmc_rprice_from_int(&a, 5);

  fmc::rprice ppa(5);
  fmc::rprice ppb(a);
  ASSERT_TRUE(fmc_rprice_equal(&a, &ppa));
  ASSERT_TRUE(fmc_rprice_equal(&a, &ppb));

  fmc::rprice ppc;
  ppa = ppa - ppa;
  ASSERT_TRUE(fmc_rprice_equal(&ppa, &ppc));
}

TEST(rprice, cppdivide) {
  fmc::rprice cppa(10);
  fmc::rprice cppb(5);
  fmc::rprice cppc(2);
  ASSERT_EQ(cppa / cppb, cppc);

  fmc_rprice_t ppa;
  fmc_rprice_from_int(&ppa, 10);
  fmc_rprice_t ppb;
  fmc_rprice_from_int(&ppb, 5);
  fmc_rprice_t ppc;
  fmc_rprice_from_int(&ppc, 2);
  ASSERT_EQ(ppa / ppb, ppc);
}

TEST(rprice, cppintdivide) {
  fmc::rprice cppa(10);
  int64_t b = 5;
  fmc::rprice cppc(2);
  ASSERT_EQ(cppa / b, cppc);
}

TEST(rprice, cppadd) {
  fmc::rprice cppa(7);
  fmc::rprice cppb(5);
  fmc::rprice cppc(2);
  ASSERT_EQ(cppc + cppb, cppa);

  fmc_rprice_t ppa;
  fmc_rprice_from_int(&ppa, 7);
  fmc_rprice_t ppb;
  fmc_rprice_from_int(&ppb, 5);
  fmc_rprice_t ppc;
  fmc_rprice_from_int(&ppc, 2);
  ASSERT_EQ(ppc + ppb, ppa);
}

TEST(rprice, cppsub) {
  fmc::rprice cppa(7);
  fmc::rprice cppb(5);
  fmc::rprice cppc(2);
  ASSERT_EQ(cppa - cppb, cppc);

  fmc_rprice_t ppa;
  fmc_rprice_from_int(&ppa, 7);
  fmc_rprice_t ppb;
  fmc_rprice_from_int(&ppb, 5);
  fmc_rprice_t ppc;
  fmc_rprice_from_int(&ppc, 2);
  ASSERT_EQ(ppa - ppb, ppc);
}

TEST(rprice, cppmul) {
  fmc::rprice cppa(10);
  fmc::rprice cppb(5);
  fmc::rprice cppc(2);
  ASSERT_EQ(cppc * cppb, cppa);

  fmc_rprice_t ppa;
  fmc_rprice_from_int(&ppa, 10);
  fmc_rprice_t ppb;
  fmc_rprice_from_int(&ppb, 5);
  fmc_rprice_t ppc;
  fmc_rprice_from_int(&ppc, 2);
  ASSERT_EQ(ppc * ppb, ppa);
}

TEST(rprice, cppcomparison) {
  fmc::rprice cppa(5);
  fmc::rprice cppb(8);
  fmc::rprice cppc(10);

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

  fmc_rprice_t ppa;
  fmc_rprice_from_int(&ppa, 5);
  fmc_rprice_t ppb;
  fmc_rprice_from_int(&ppb, 8);
  fmc_rprice_t ppc;
  fmc_rprice_from_int(&ppc, 10);

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

TEST(rprice, cppincrement) {
  fmc::rprice cppa(5);
  fmc::rprice cppb(10);
  cppa += cppa;
  ASSERT_EQ(cppa, cppb);
}

TEST(rprice, cppdecrement) {
  fmc::rprice cppa(10);
  fmc::rprice cppb(5);
  cppa -= cppb;
  ASSERT_EQ(cppa, cppb);
}

TEST(rprice, cppupcasting) {
  fmc_rprice_t a;
  fmc::rprice &ppa = fmc::rprice::upcast(a);
  ASSERT_EQ(&a, &ppa);
}

TEST(rprice, cppimplicit_downcasting) {
  fmc::rprice ppa(5);
  auto f = [](const fmc::rprice &lhs, const fmc_rprice_t &rhs) -> bool {
    return &lhs == &rhs;
  };
  ASSERT_TRUE(f(ppa, ppa));
}

TEST(rprice, cppnegate) {
  fmc::rprice a(4);
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));
  ASSERT_TRUE(std::isfinite(a));

  fmc::rprice b = -a;

  ASSERT_NE(a, b);
  ASSERT_EQ(a, -b);
  ASSERT_EQ(a, std::abs(b));
}

TEST(rprice, ostream) {
  fmc::rprice a(5);
  a = a / (int64_t)10;
  std::ostringstream str;
  str << a;
  std::string res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "0.5");

  str.str("");
  str.clear();
  fmc_rprice_t c;
  fmc_rprice_from_int(&c, 5);
  str << c;
  res = str.str();
  ASSERT_EQ(res.size(), 1);
  ASSERT_STREQ(res.c_str(), "5");
}

TEST(rprice, cppmax) {
  fmc::rprice a = std::numeric_limits<fmc::rprice>::max();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "9223372036.85478");
}

TEST(rprice, cppmin) {
  fmc::rprice a = std::numeric_limits<fmc::rprice>::min();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "-9223372036.85478");
}

TEST(rprice, cppepsilon) {
  fmc::rprice a = std::numeric_limits<fmc::rprice>::epsilon();
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "0");
}

TEST(rprice, cppdecimalfromint) {
  fmc::rprice a = 5;
  fmc::rprice b(5);
  int64_t ic = 5;
  fmc::rprice c(ic);
  ASSERT_EQ(a, b);
  ASSERT_EQ(a, c);
  fmc::rprice u = 5U;
  uint64_t ud = 5;
  fmc::rprice d(ud);
  ASSERT_EQ(a, u);
  std::ostringstream str;
  str << a;
  ASSERT_STREQ(str.str().c_str(), "5");
}

TEST(rprice, cppstreams) {
  std::string s("4323255553");
  std::stringstream ss(s);
  fmc::rprice a((int64_t)4323255553);
  fmc::rprice b;
  ss >> b;
  ASSERT_EQ(a, b);
  fmc::rprice c((uint64_t)4323255553);
  ASSERT_EQ(c, b);
}

TEST(rprice, invalid_cppstreams) {
  std::string s("invaliddecimal");
  std::stringstream ss(s);
  fmc::rprice b;
  ASSERT_THROW(ss >> b, std::runtime_error);
}

TEST(rprice, assign) {
  fmc::rprice a = 5;
  fmc::rprice b = a;
  ASSERT_EQ(a, b);
}

TEST(rprice, move) {
  fmc::rprice a = 5;
  fmc::rprice b = std::move(a);
  fmc::rprice c = 5;
  ASSERT_EQ(b, c);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
