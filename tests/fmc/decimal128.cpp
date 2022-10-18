
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
  uint64_t max = 18446744073709551615;
  ASSERT_EQ(max, std::numeric_limits<uint64_t>::max());
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
  int64_t max = 9223372036854775807;
  ASSERT_EQ(max, std::numeric_limits<int64_t>::max());
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
  int64_t max = -9223372036854775808;
  ASSERT_EQ(max, std::numeric_limits<int64_t>::min());
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

// C++ API
TEST(decimal128, cppconstructor) {
  fmc_decimal128_t a;
  fmc_decimal128_from_str(&a, "5");

  fmc::decimal128 ppa(5);
  fmc::decimal128 ppb(a);
  ASSERT_TRUE(fmc_decimal128_equal(a, ppa));
  ASSERT_TRUE(fmc_decimal128_equal(a, ppb));
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

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
