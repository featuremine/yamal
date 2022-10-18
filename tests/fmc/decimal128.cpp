
#include "fmc/decimal128.h"
#include <fmc++/gtestwrap.hpp>
#include <string.h>

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
