
#include "fmc/decimal128.h"
#include "fmc++/decimal128.hpp"
#include <fmc++/gtestwrap.hpp>
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

// C++ API
TEST(decimal128, cppconstructor) {
  fmc_decimal128_t a;
  fmc_decimal128_from_str(&a, "5");

  fmc::decimal128 ppa(5);
  fmc::decimal128 ppb(a);
  ASSERT_TRUE(fmc_decimal128_equal(a, ppa));
  ASSERT_TRUE(fmc_decimal128_equal(a, ppb));
}

TEST(decimal128, cppdivide) {
  fmc::decimal128 ppa(10);
  fmc::decimal128 ppb(5);
  fmc::decimal128 ppc(2);
  ASSERT_EQ(ppa / ppb, ppc);
}

TEST(decimal128, cppint_div) {
  fmc::decimal128 ppa(10);
  fmc::decimal128 ppb(2);
  ASSERT_EQ(ppa / 5, ppb);
}

TEST(decimal128, cppadd) {
  fmc::decimal128 ppa(7);
  fmc::decimal128 ppb(5);
  fmc::decimal128 ppc(2);
  ASSERT_EQ(ppc + ppb, ppa);
}

TEST(decimal128, cppsub) {
  fmc::decimal128 ppa(7);
  fmc::decimal128 ppb(5);
  fmc::decimal128 ppc(2);
  ASSERT_EQ(ppa - ppb, ppc);
}

TEST(decimal128, cppmul) {
  fmc::decimal128 ppa(10);
  fmc::decimal128 ppb(5);
  fmc::decimal128 ppc(2);
  ASSERT_EQ(ppc * ppb, ppa);
}

TEST(decimal128, cppcomparison) {
  fmc::decimal128 ppa(5);
  fmc::decimal128 ppb(8);
  fmc::decimal128 ppc(10);

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
  fmc::decimal128 ppa(5);
  fmc::decimal128 ppb(10);
  ppa += ppa;
  ASSERT_EQ(ppa, ppb);
}

TEST(decimal128, cppdecrement) {
  fmc::decimal128 ppa(10);
  fmc::decimal128 ppb(5);
  ppa -= ppb;
  ASSERT_EQ(ppa, ppb);
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

TEST(decimal128, cppinfinity) {
  fmc::decimal128 a(4);
  ASSERT_FALSE(fmc_decimal128_is_inf(a));

  fmc::decimal128 inf = std::numeric_limits<fmc::decimal128>::infinity();
  ASSERT_TRUE(fmc_decimal128_is_inf(inf));
}

TEST(decimal128, cppnan) {
  fmc::decimal128 a(4);
  ASSERT_FALSE(fmc_decimal128_is_nan(a));
  ASSERT_FALSE(fmc_decimal128_is_qnan(a));
  ASSERT_FALSE(fmc_decimal128_is_snan(a));

  fmc::decimal128 qnan = std::numeric_limits<fmc::decimal128>::quiet_NaN();
  ASSERT_TRUE(fmc_decimal128_is_nan(qnan));
  ASSERT_TRUE(fmc_decimal128_is_qnan(qnan));
  ASSERT_FALSE(fmc_decimal128_is_snan(qnan));
  ASSERT_FALSE(fmc_decimal128_is_inf(qnan));

  fmc::decimal128 snan = std::numeric_limits<fmc::decimal128>::signaling_NaN();
  ASSERT_TRUE(fmc_decimal128_is_nan(snan));
  ASSERT_FALSE(fmc_decimal128_is_qnan(snan));
  ASSERT_TRUE(fmc_decimal128_is_snan(snan));
  ASSERT_FALSE(fmc_decimal128_is_inf(snan));
}

TEST(decimal128, ostream) {
  fmc::decimal128 a(5);
  a = a / 10;
  std::ostringstream str;
  str << a;
  std::string res = str.str();
  ASSERT_EQ(res.size(), 3);
  ASSERT_STREQ(res.c_str(), "0.5");

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
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
