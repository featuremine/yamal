
#include "fmc/decimal128.h"
#include <fmc++/gtestwrap.hpp>
#include <string.h>

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
  fmc_decimal128_from_str(&b, "2");
  a = fmc_decimal128_divide(a, b);
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

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
