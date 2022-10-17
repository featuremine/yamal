
#include "fmc/decimal128.h"
#include <string.h>

int main(int argc, char **argv) {
  fmc_decimal128_t a, b;
  char str[256];
  fm_decimal128_from_str(&a, "11111111111.11111111111111111");
  fm_decimal128_from_str(&b, "22222222222.22222222222222222");
  fm_decimal128_add(&a, &a, &b);
  fm_decimal128_to_str(&a, str);
  ASSERT_STREQ(str, "33333333333.33333333333333333");
  return 0;
}
