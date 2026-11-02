#include <criterion/criterion.h>
#include <stdint.h>
#include <math.h> /* for isnan */

uint32_t float_to_bits(float f) {
  union {
    float f;
    uint32_t u;
  } u1;
  u1.f = f;
  return u1.u;
}

float bits_to_float(uint32_t u) {
  union {
    float f;
    uint32_t u;
  } u2;
  u2.u = u;
  return u2.f;
}

Test(single_precision_float, float_to_bits_and_back_1f) {
  float f1 = 1.0f;
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x3f800000, "float_to_bits(1.0f) should be 0x3f800000");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(1.0f)) should be 1.0f");
}

Test(single_precision_float, float_to_bits_and_back_0f) {
  float f1 = 0.0f;
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x00000000, "float_to_bits(0.0f) should be 0x00000000");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(0.0f)) should be 0.0f");
}

Test(single_precision_float, float_to_bits_and_back_negative_1f) {
  float f1 = -1.0f;
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0xbf800000, "float_to_bits(-1.0f) should be 0xbf800000");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(-1.0f)) should be -1.0f");
}

Test(single_precision_float, float_to_bits_and_back_pi) {
  float f1 = 3.14159265f;
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x40490fdb, "float_to_bits(3.14159265f) should be 0x40490fdb");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(3.14159265f)) should be 3.14159265f");
}

Test(single_precision_float, float_to_bits_and_back_infinity) {
  float f1 = INFINITY;
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x7f800000, "float_to_bits(infinity) should be 0x7f800000");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(infinity)) should be infinity");
}

Test(single_precision_float, float_to_bits_and_back_nan) {
  float f1 = NAN;
  uint32_t bits = float_to_bits(f1);
  cr_assert((bits & 0x7f800000) == 0x7f800000 && (bits & 0x007fffff) != 0, "float_to_bits(NaN) should have exponent bits all ones and non-zero mantissa");
  float f2 = bits_to_float(bits);
  cr_assert(isnan(f2), "bits_to_float(float_to_bits(NaN)) should be NaN");
}

Test(single_precision_float, float_to_bits_and_back_smallest_positive) {
  float f1 = 1.17549435e-38f; /* Smallest positive normal float */
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x00800000, "float_to_bits(smallest positive normal float) should be 0x00800000");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(smallest positive normal float)) should be the same value");
}

Test(single_precision_float, float_to_bits_and_back_largest) {
  float f1 = 3.40282347e+38f; /* Largest positive normal float */
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x7f7fffff, "float_to_bits(largest positive normal float) should be 0x7f7fffff");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(largest positive normal float)) should be the same value");
}

Test(single_precision_float, float_to_bits_and_back_subnormal) {
  float f1 = 1.40129846e-45f; /* Smallest positive subnormal float */
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x00000001, "float_to_bits(smallest positive subnormal float) should be 0x00000001");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(smallest positive subnormal float)) should be the same value");
}

Test(single_precision_float, float_to_bits_and_back_negative_subnormal) {
  float f1 = -1.40129846e-45f; /* Smallest negative subnormal float */
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x80000001, "float_to_bits(smallest negative subnormal float) should be 0x80000001");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(smallest negative subnormal float)) should be the same value");
}

Test(single_precision_float, float_to_bits_and_back_large_negative) {
  float f1 = -3.40282347e+38f; /* Largest negative normal float */
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0xff7fffff, "float_to_bits(largest negative normal float) should be 0xff7fffff");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(largest negative normal float)) should be the same value");
}

Test(single_precision_float, float_to_bits_and_back_large_positive) {
  float f1 = 3.40282347e+38f; /* Largest positive normal float */
  uint32_t bits = float_to_bits(f1);
  cr_assert_eq(bits, 0x7f7fffff, "float_to_bits(largest positive normal float) should be 0x7f7fffff");
  float f2 = bits_to_float(bits);
  cr_assert_eq(f2, f1, "bits_to_float(float_to_bits(largest positive normal float)) should be the same value");
}

uint64_t double_to_bits(double d) {
  union {
    double d;
    uint64_t u;
  } u1;
  u1.d = d;
  return u1.u;
}

double bits_to_double(uint64_t u) {
  union {
    double d;
    uint64_t u;
  } u2;
  u2.u = u;
  return u2.d;
}

Test(double_precision_float, double_to_bits_and_back_1d) {
  double d1 = 1.0;
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x3ff0000000000000ULL, "double_to_bits(1.0) should be 0x3ff0000000000000");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(1.0)) should be 1.0");
}

Test(double_precision_float, double_to_bits_and_back_0d) {
  double d1 = 0.0;
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x0000000000000000ULL, "double_to_bits(0.0) should be 0x0000000000000000");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(0.0)) should be 0.0");
}

Test(double_precision_float, double_to_bits_and_back_negative_1d) {
  double d1 = -1.0;
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0xbff0000000000000ULL, "double_to_bits(-1.0) should be 0xbff0000000000000");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(-1.0)) should be -1.0");
}

Test(double_precision_float, double_to_bits_and_back_pi) {
  double d1 = 3.141592653589793;
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x400921fb54442d18ULL, "double_to_bits(3.141592653589793) should be 0x400921fb54442d18");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(3.141592653589793)) should be 3.141592653589793");
}

Test(double_precision_float, double_to_bits_and_back_infinity) {
  double d1 = INFINITY;
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x7ff0000000000000ULL, "double_to_bits(infinity) should be 0x7ff0000000000000");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(infinity)) should be infinity");
}

Test(double_precision_float, double_to_bits_and_back_nan) {
  double d1 = NAN;
  uint64_t bits = double_to_bits(d1);
  cr_assert((bits & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL && (bits & 0x000fffffffffffffULL) != 0, "double_to_bits(NaN) should have exponent bits all ones and non-zero mantissa");
  double d2 = bits_to_double(bits);
  cr_assert(isnan(d2), "bits_to_double(double_to_bits(NaN)) should be NaN");
}

Test(double_precision_float, double_to_bits_and_back_smallest_positive) {
  double d1 = 2.2250738585072014e-308; /* Smallest positive normal double */
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x0010000000000000ULL, "double_to_bits(smallest positive normal double) should be 0x0010000000000000");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(smallest positive normal double)) should be the same value");
}

Test(double_precision_float, double_to_bits_and_back_largest) {
  double d1 = 1.7976931348623157e+308; /* Largest positive normal double */
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x7fefffffffffffffULL, "double_to_bits(largest positive normal double) should be 0x7fefffffffffffff");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(largest positive normal double)) should be the same value");
}

Test(double_precision_float, double_to_bits_and_back_subnormal) {
  double d1 = 4.9406564584124654e-324; /* Smallest positive subnormal double */
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x0000000000000001ULL, "double_to_bits(smallest positive subnormal double) should be 0x0000000000000001");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(smallest positive subnormal double)) should be the same value");
}

Test(double_precision_float, double_to_bits_and_back_negative_subnormal) {
  double d1 = -4.9406564584124654e-324; /* Smallest negative subnormal double */
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x8000000000000001ULL, "double_to_bits(smallest negative subnormal double) should be 0x8000000000000001");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(smallest negative subnormal double)) should be the same value");
}

Test(double_precision_float, double_to_bits_and_back_large_negative) {
  double d1 = -1.7976931348623157e+308; /* Largest negative normal double */
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0xffefffffffffffffULL, "double_to_bits(largest negative normal double) should be 0xffefffffffffffff");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(largest negative normal double)) should be the same value");
}

Test(double_precision_float, double_to_bits_and_back_large_positive) {
  double d1 = 1.7976931348623157e+308; /* Largest positive normal double */
  uint64_t bits = double_to_bits(d1);
  cr_assert_eq(bits, 0x7fefffffffffffffULL, "double_to_bits(largest positive normal double) should be 0x7fefffffffffffff");
  double d2 = bits_to_double(bits);
  cr_assert_eq(d2, d1, "bits_to_double(double_to_bits(largest positive normal double)) should be the same value");
}