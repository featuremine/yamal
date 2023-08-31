/******************************************************************************
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

/**
 * @file rational.hpp
 * @author Andres Rangel
 * @date 8 Jan 2019
 * @brief File contains tests for rational serialization
 *
 * @see http://www.featuremine.com
 */

#include "fmc++/rational64.hpp"
#include "fmc++/rprice.hpp"

#include "fmc++/gtestwrap.hpp"
#include <cmath>
#include <sstream>

TEST(rational, api) {
  fmc_rational64_t zero;
  fmc_rational64_zero(&zero);
  ASSERT_EQ(zero.num, 0);
  ASSERT_EQ(zero.den, 1);

  fmc_rational64_t sample;
  fmc_rational64_new(&sample, 31, 32);
  ASSERT_EQ(sample.num, 31);
  ASSERT_EQ(sample.den, 32);

  double val;
  fmc_rational64_to_double(&val, &sample);
  ASSERT_DOUBLE_EQ(val, 0.96875);

  fmc_rational64_t sample_from_double;
  fmc_rational64_from_double(&sample_from_double, val);
  ASSERT_EQ(sample_from_double.num, 31);
  ASSERT_EQ(sample_from_double.den, 32);

  ASSERT_TRUE(fmc_rational64_equal(&sample, &sample_from_double));

  fmc_rational64_t sample_from_int;
  fmc_rational64_from_int(&sample_from_int, 68);
  ASSERT_EQ(sample_from_int.num, 68);
  ASSERT_EQ(sample_from_int.den, 1);

  ASSERT_TRUE(fmc_rational64_less(&zero, &sample));
  ASSERT_TRUE(zero < sample);
  ASSERT_TRUE(zero <= sample);

  ASSERT_TRUE(zero <= zero);
  ASSERT_TRUE(zero >= zero);

  ASSERT_TRUE(fmc_rational64_greater(&sample_from_int, &sample));
  ASSERT_TRUE(sample_from_int > sample);
  ASSERT_TRUE(sample_from_int >= sample);

  ASSERT_TRUE(fmc_rational64_notequal(&sample_from_int, &sample_from_double));
  ASSERT_TRUE(sample_from_int != sample_from_double);

  fmc_rational64_t res_div;
  fmc_rational64_div(&res_div, &sample_from_int, &sample_from_double);
  ASSERT_EQ(res_div.num, 2176);
  ASSERT_EQ(res_div.den, 31);
  ASSERT_EQ(res_div, sample_from_int / sample_from_double);

  fmc_rational64_t inf_div;
  fmc_rational64_div(&inf_div, &sample_from_int, &zero);
  ASSERT_EQ(inf_div.num, 1);
  ASSERT_EQ(inf_div.den, 0);
  ASSERT_TRUE(inf_div == sample_from_int / zero);
  ASSERT_TRUE(std::isinf(inf_div));
  ASSERT_FALSE(std::isfinite(inf_div));
  ASSERT_FALSE(std::isnan(inf_div));

  fmc_rational64_t res_add;
  fmc_rational64_add(&res_add, &sample, &sample_from_double);
  ASSERT_EQ(res_add.num, 31);
  ASSERT_EQ(res_add.den, 16);
  ASSERT_EQ(res_add, sample + sample_from_double);

  fmc_rational64_t res_mul;
  fmc_rational64_mul(&res_mul, &sample_from_int, &sample_from_double);
  ASSERT_EQ(res_mul.num, 527);
  ASSERT_EQ(res_mul.den, 8);
  ASSERT_EQ(res_mul, sample_from_int * sample_from_double);

  fmc_rational64_t res_sub;
  fmc_rational64_sub(&res_sub, &res_add, &res_mul);
  ASSERT_EQ(res_sub.num, -1023);
  ASSERT_EQ(res_sub.den, 16);
  ASSERT_EQ(res_sub, res_add - res_mul);

  fmc_rational64_t nan;
  fmc_rational64_nan(&nan);
  ASSERT_EQ(nan.num, 0);
  ASSERT_EQ(nan.den, 0);
  ASSERT_FALSE(std::isinf(nan));
  ASSERT_FALSE(std::isfinite(nan));
  ASSERT_TRUE(std::isnan(nan));

  ASSERT_TRUE(fmc_rational64_is_nan(&nan));
  ASSERT_FALSE(fmc_rational64_is_nan(&res_div));

  fmc_rational64_t inf;
  fmc_rational64_inf(&inf);
  ASSERT_EQ(inf.num, 1);
  ASSERT_EQ(inf.den, 0);
  ASSERT_TRUE(std::isinf(inf));
  ASSERT_FALSE(std::isfinite(inf));
  ASSERT_FALSE(std::isnan(inf));

  ASSERT_TRUE(fmc_rational64_is_inf(&inf));
  ASSERT_TRUE(fmc_rational64_is_inf(&inf_div));
  ASSERT_FALSE(fmc_rational64_is_inf(&res_sub));
}

TEST(rational, decimal_conversions) {
  double val = -9.0 - (31.0 / 32.0);
  fmc::rprice d, tmpd;
  fmc_rprice_from_double(&d, val);
  fmc::rational64 r, tmpr;
  fmc_rational64_from_double(&r, val);
  fmc_rational64_from_rprice(&tmpr, &d);
  ASSERT_EQ(r, tmpr);
  fmc_rational64_to_rprice(&tmpd, &r);
  ASSERT_EQ(d, tmpd);
  double dr;
  fmc_rational64_to_double(&dr, &r);
  ASSERT_DOUBLE_EQ(dr, val);
  double res;
  fmc_rprice_to_double(&res, &d);
  ASSERT_DOUBLE_EQ(res, val);
}

TEST(rational, serialization) {
  fmc::rational64 a;
  fmc_rational64_new(&a, 4, 5);
  std::stringstream s1;
  s1 << a;
  ASSERT_EQ(s1.str().compare("4/5"), 0);
  std::stringstream s2;
  s2 << "6/8";
  s2 >> a;
  std::stringstream s3;
  s3 << a;
  ASSERT_EQ(s3.str().compare("6/8"), 0);
  std::stringstream s4;
  s4 << "3.257";
  s4 >> a;
  std::stringstream s5;
  s5 << a;
  std::stringstream s6;
  double d;
  fmc_rational64_to_double(&d, &a);
  s6 << d;
  ASSERT_EQ(s5.str().compare("3257/1000"), 0);
  ASSERT_EQ(s6.str().compare("3.257"), 0);
}

TEST(rational64, cppnegate) {
  fmc::rational64 a(4);
  ASSERT_FALSE(std::isinf(a));
  ASSERT_FALSE(std::isnan(a));
  ASSERT_TRUE(std::isfinite(a));

  fmc::rational64 b = -a;

  ASSERT_NE(a, b);
  ASSERT_EQ(a, -b);
  ASSERT_EQ(a, std::abs(b));

  fmc::rational64 c(-4);

  ASSERT_EQ(a, -c);
  ASSERT_EQ(b, c);
  ASSERT_EQ(a, std::abs(c));
}

TEST(rational64, cppincdec) {
  fmc::rational64 a(4);
  a += fmc::rational64(2);
  ASSERT_EQ(a, fmc::rational64(6));
  a -= fmc::rational64(4);
  ASSERT_EQ(a, fmc::rational64(2));
}

TEST(rational64, doubleconersions) {
  double a = std::numeric_limits<double>::infinity();
  fmc::rational64 ra(a);
  ASSERT_DOUBLE_EQ(double(ra), a);

  a = -std::numeric_limits<double>::infinity();
  ra = fmc::rational64(a);
  ASSERT_DOUBLE_EQ(double(ra), a);

  a = std::numeric_limits<double>::quiet_NaN();
  ra = fmc::rational64(a);
  ASSERT_TRUE(std::isnan(a));
  ASSERT_TRUE(std::isnan(double(ra)));

  a = -std::numeric_limits<double>::quiet_NaN();
  ra = fmc::rational64(a);
  ASSERT_TRUE(std::isnan(a));
  ASSERT_TRUE(std::isnan(double(ra)));

  a = -9.0;
  ra = fmc::rational64(a);
  ASSERT_DOUBLE_EQ(double(ra), a);

  a = 22.0;
  ra = fmc::rational64(a);
  ASSERT_DOUBLE_EQ(double(ra), a);

  a = 0.0;
  ra = fmc::rational64(a);
  ASSERT_DOUBLE_EQ(double(ra), a);
}

TEST(rational64, numeric_limits) {
  fmc_rational64_t val;
  fmc_rational64_inf(&val);
  ASSERT_EQ(std::numeric_limits<fmc::rational64>::infinity(), val);
  ASSERT_FALSE(std::isfinite(val));
  ASSERT_TRUE(std::isinf(val));
  ASSERT_FALSE(std::isnan(val));

  val = fmc::rational64(0.5);
  ASSERT_EQ(std::numeric_limits<fmc::rational64>::round_error(), val);
  ASSERT_TRUE(std::isfinite(val));
  ASSERT_FALSE(std::isinf(val));
  ASSERT_FALSE(std::isnan(val));

  val = fmc::rational64(-std::numeric_limits<fmc::rational64>::infinity());
  ASSERT_EQ(-std::numeric_limits<fmc::rational64>::infinity(), val);
  ASSERT_FALSE(std::isfinite(val));
  ASSERT_TRUE(std::isinf(val));
  ASSERT_FALSE(std::isnan(val));

  fmc_rational64_nan(&val);
  ASSERT_EQ(std::numeric_limits<fmc::rational64>::quiet_NaN(), val);
  ASSERT_FALSE(std::isfinite(val));
  ASSERT_FALSE(std::isinf(val));
  ASSERT_TRUE(std::isnan(val));

  fmc_rational64_min(&val);
  ASSERT_EQ(std::numeric_limits<fmc::rational64>::min(), val);
  ASSERT_EQ(std::numeric_limits<fmc::rational64>::lowest(), val);
  ASSERT_TRUE(std::isfinite(val));
  ASSERT_FALSE(std::isinf(val));
  ASSERT_FALSE(std::isnan(val));

  fmc_rational64_max(&val);
  ASSERT_EQ(std::numeric_limits<fmc::rational64>::max(), val);
  ASSERT_TRUE(std::isfinite(val));
  ASSERT_FALSE(std::isinf(val));
  ASSERT_FALSE(std::isnan(val));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
