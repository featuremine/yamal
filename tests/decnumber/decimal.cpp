#include "fmc++/gtestwrap.hpp"

#include "decQuad.h"

TEST(decimal, instantiation) {
  decQuad a, b;
  decContext set;
  char string[256];
  decContextDefault(&set, DEC_INIT_DECQUAD);
  decQuadFromString(&a, "11111111111.11111111111111111", &set);
  decQuadFromString(&b, "22222222222.22222222222222222", &set);
  decQuadAdd(&a, &a, &b, &set);
  decQuadToString(&a, string);
  EXPECT_STREQ(string, "33333333333.33333333333333333");
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
