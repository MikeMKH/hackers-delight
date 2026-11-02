#include <criterion/criterion.h>
#include <stdint.h>
#include <math.h> /* for isnan */
#include <string.h> /* memcpy */

uint32_t float_to_bits(float f) {
  uint32_t u;
  memcpy(&u, &f, sizeof u);
  return u;
}

float bits_to_float(uint32_t u) {
  float f;
  memcpy(&f, &u, sizeof f);
  return f;
}

uint64_t double_to_bits(double d) {
  uint64_t u;
  memcpy(&u, &d, sizeof u);
  return u;
}

double bits_to_double(uint64_t u) {
  double d;
  memcpy(&d, &u, sizeof d);
  return d;
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

typedef struct { const char *name; float v; uint32_t bits; } f32_case;
typedef struct { const char *name; double v; uint64_t bits; } f64_case;

static const f32_case f32_cases[] = {
  {"+0",            0.0f,             0x00000000u},
  {"-0",           -0.0f,             0x80000000u},
  {"1.0",           1.0f,             0x3f800000u},
  {"-2.0",         -2.0f,             0xc0000000u},
  {"0.5",           0.5f,             0x3f000000u},
  {"0.1 (inexact)", 0.1f,             0x3dcccccdu},
  {"pi",            3.14159265f,      0x40490fdbu},
  {"min normal",    0x1p-126f,        0x00800000u},
  {"max subnormal", 0x1.fffffcp-127f, 0x007fffffu},
  {"min subnormal", 0x1p-149f,        0x00000001u},
  {"max normal",    0x1.fffffep+127f, 0x7f7fffffu},
  {"+inf",          INFINITY,         0x7f800000u},
  {"-inf",         -INFINITY,         0xff800000u},
};

static const f64_case f64_cases[] = {
  {"+0",            0.0,                     0x0000000000000000ull},
  {"-0",           -0.0,                     0x8000000000000000ull},
  {"1.0",           1.0,                     0x3ff0000000000000ull},
  {"-2.0",         -2.0,                     0xc000000000000000ull},
  {"0.5",           0.5,                     0x3fe0000000000000ull},
  {"0.1 (inexact)", 0.1,                     0x3fb999999999999aull},
  {"pi",            3.141592653589793,       0x400921fb54442d18ull},
  {"min normal",    0x1p-1022,               0x0010000000000000ull},
  {"max subnormal", 0x1.ffffffffffffep-1023, 0x000fffffffffffffull},
  {"min subnormal", 0x1p-1074,               0x0000000000000001ull},
  {"max normal",    0x1.fffffffffffffp+1023, 0x7fefffffffffffffull},
  {"+inf",          (double)INFINITY,        0x7ff0000000000000ull},
  {"-inf",         -(double)INFINITY,        0xfff0000000000000ull},
};

#define COUNT(a) (sizeof (a) / sizeof *(a))

Test(single_precision_float, known_encodings) {
  for (size_t i = 0; i < COUNT(f32_cases); i++) {
    uint32_t got = float_to_bits(f32_cases[i].v);
    cr_assert_eq(got, f32_cases[i].bits, "%s: got 0x%08x, want 0x%08x",
                 f32_cases[i].name, got, f32_cases[i].bits);
    uint32_t rt = float_to_bits(bits_to_float(f32_cases[i].bits));
    cr_assert_eq(rt, f32_cases[i].bits, "%s: round trip 0x%08x", f32_cases[i].name, rt);
  }
}

Test(double_precision_float, known_encodings) {
  for (size_t i = 0; i < COUNT(f64_cases); i++) {
    uint64_t got = double_to_bits(f64_cases[i].v);
    cr_assert_eq(got, f64_cases[i].bits, "%s: got 0x%016llx, want 0x%016llx",
                 f64_cases[i].name, (unsigned long long)got, (unsigned long long)f64_cases[i].bits);
    uint64_t rt = double_to_bits(bits_to_double(f64_cases[i].bits));
    cr_assert_eq(rt, f64_cases[i].bits, "%s: round trip 0x%016llx",
                 f64_cases[i].name, (unsigned long long)rt);
  }
}

Test(single_precision_float, fields_of_minus_6_25) {
  uint32_t b = float_to_bits(-6.25f);
  cr_assert_eq(b >> 31, 1u);                 /* sign */
  cr_assert_eq((b >> 23) & 0xff, 129u);      /* 127 + 2 */
  cr_assert_eq(b & 0x7fffff, 0x480000u);     /* .1001 0000... */
  cr_assert_eq(b, 0xc0c80000u);
}

Test(double_precision_float, fields_of_minus_6_25) {
  uint64_t b = double_to_bits(-6.25);
  cr_assert_eq(b >> 63, 1u);
  cr_assert_eq((b >> 52) & 0x7ff, 1025u);    /* 1023 + 2 */
  cr_assert_eq(b & 0xfffffffffffffull, 0x9000000000000ull);
  cr_assert_eq(b, 0xc019000000000000ull);
}

Test(single_precision_float, nan_payloads_preserved) {
  static const uint32_t nans[] = {0x7fc00000u, 0xffc00000u, 0x7fc12345u};  /* +qNaN, -qNaN, qNaN with payload */
  for (size_t i = 0; i < COUNT(nans); i++) {
    float f = bits_to_float(nans[i]);
    cr_assert(isnan(f));
    cr_assert_eq(float_to_bits(f), nans[i], "payload lost for 0x%08x", nans[i]);
  }
}