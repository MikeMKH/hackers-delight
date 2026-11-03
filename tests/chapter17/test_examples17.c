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

#define F32_BITS(s, e, m) (((uint32_t)(s) << 31) | ((uint32_t)(e) << 23) | (uint32_t)(m))
#define F64_BITS(s, e, m) (((uint64_t)(s) << 63) | ((uint64_t)(e) << 52) | (uint64_t)(m))
#define COUNT(a) (sizeof (a) / sizeof *(a))

typedef struct { const char *name; float v;  uint32_t sign, exp, mant; } f32_case;
typedef struct { const char *name; double v; uint64_t sign, exp, mant; } f64_case;

static const f32_case f32_cases[] = {
  /* name             value             s  exp    mantissa (23 bits) */
  {"+0",              0.0f,             0, 0x00,  0x000000},
  {"-0",             -0.0f,             1, 0x00,  0x000000},
  {"1.0",             1.0f,             0, 0x7f,  0x000000},
  {"-2.0",           -2.0f,             1, 0x80,  0x000000},
  {"0.5",             0.5f,             0, 0x7e,  0x000000},
  {"0.1 (inexact)",   0.1f,             0, 0x7b,  0x4ccccd},
  {"pi",              3.14159265f,      0, 0x80,  0x490fdb},
  {"min normal",      0x1p-126f,        0, 0x01,  0x000000},
  {"max subnormal",   0x1.fffffcp-127f, 0, 0x00,  0x7fffff},
  {"min subnormal",   0x1p-149f,        0, 0x00,  0x000001},
  {"max normal",      0x1.fffffep+127f, 0, 0xfe,  0x7fffff},
  {"+inf",            INFINITY,         0, 0xff,  0x000000},
  {"-inf",           -INFINITY,         1, 0xff,  0x000000},
};

static const f64_case f64_cases[] = {
  /* name             value                    s  exp     mantissa (52 bits) */
  {"+0",              0.0,                     0, 0x000,  0x0000000000000},
  {"-0",             -0.0,                     1, 0x000,  0x0000000000000},
  {"1.0",             1.0,                     0, 0x3ff,  0x0000000000000},
  {"-2.0",           -2.0,                     1, 0x400,  0x0000000000000},
  {"0.5",             0.5,                     0, 0x3fe,  0x0000000000000},
  {"0.1 (inexact)",   0.1,                     0, 0x3fb,  0x999999999999a},
  {"pi",              3.141592653589793,       0, 0x400,  0x921fb54442d18},
  {"min normal",      0x1p-1022,               0, 0x001,  0x0000000000000},
  {"max subnormal",   0x1.ffffffffffffep-1023, 0, 0x000,  0xfffffffffffff},
  {"min subnormal",   0x1p-1074,               0, 0x000,  0x0000000000001},
  {"max normal",      0x1.fffffffffffffp+1023, 0, 0x7fe,  0xfffffffffffff},
  {"+inf",            (double)INFINITY,        0, 0x7ff,  0x0000000000000},
  {"-inf",           -(double)INFINITY,        1, 0x7ff,  0x0000000000000},
};

Test(single_precision_float, known_encodings) {
  for (size_t i = 0; i < COUNT(f32_cases); i++) {
    const f32_case *c = &f32_cases[i];
    cr_assert(c->sign <= 1 && c->exp <= 0xff && c->mant <= 0x7fffff, "%s: field out of range", c->name);

    uint32_t want = F32_BITS(c->sign, c->exp, c->mant);
    uint32_t got  = float_to_bits(c->v);
    cr_assert_eq(got, want, "%s: float_to_bits got 0x%08x, want 0x%08x", c->name, got, want);

    uint32_t rt = float_to_bits(bits_to_float(want));
    cr_assert_eq(rt, want, "%s: bits_to_float round trip got 0x%08x", c->name, rt);
  }
}

Test(double_precision_float, known_encodings) {
  for (size_t i = 0; i < COUNT(f64_cases); i++) {
    const f64_case *c = &f64_cases[i];
    cr_assert(c->sign <= 1 && c->exp <= 0x7ff && c->mant <= 0xfffffffffffffull, "%s: field out of range", c->name);
    
    uint64_t want = F64_BITS(c->sign, c->exp, c->mant);
    uint64_t got  = double_to_bits(c->v);
    cr_assert_eq(got, want, "%s: double_to_bits got 0x%016llx, want 0x%016llx",
      c->name, (unsigned long long)got, (unsigned long long)want);
    
    uint64_t rt = double_to_bits(bits_to_double(want));
    cr_assert_eq(rt, want, "%s: bits_to_double round trip got 0x%016llx", c->name, (unsigned long long)rt);
  }
}
  
Test(single_precision_float, nan_payloads_preserved) {
  static const uint32_t nans[] = {0x7fc00000u, 0xffc00000u, 0x7fc12345u}; /* +qNaN, -qNaN, qNaN */
  for (size_t i = 0; i < COUNT(nans); i++) {
    float f = bits_to_float(nans[i]);
    cr_assert(isnan(f));
    cr_assert_eq(float_to_bits(f), nans[i], "payload lost for 0x%08x", nans[i]);
  }
}

Test(single_precision_float, fields_of_minus_6_25) {
  uint32_t b = float_to_bits(-6.25f);
  cr_assert_eq(b >> 31, 1u);             /* sign */
  cr_assert_eq((b >> 23) & 0xff, 129u);  /* 127 + 2 */
  cr_assert_eq(b & 0x7fffff, 0x480000u); /* .1001 0000... */
  cr_assert_eq(b, 0xc0c80000u);
}

Test(double_precision_float, fields_of_minus_6_25) {
  uint64_t b = double_to_bits(-6.25);
  cr_assert_eq(b >> 63, 1u);
  cr_assert_eq((b >> 52) & 0x7ff, 1025u); /* 1023 + 2 */
  cr_assert_eq(b & 0xfffffffffffffull, 0x9000000000000ull);
  cr_assert_eq(b, 0xc019000000000000ull);
}

#include <fenv.h>   /* fesetround */

#ifndef COUNT
#define COUNT(a) (sizeof (a) / sizeof *(a))
#endif

/* ---- Table 17-2 constants ------------------------------------------------ */
static double c521d(void) { return bits_to_double(0x4338000000000000ULL); } /* 2^52 + 2^51 */
static double c52d(void)  { return bits_to_double(0x4330000000000000ULL); } /* 2^52        */
static float  c231f(void) { return bits_to_float(0x4b400000u); }            /* 2^23 + 2^22 */
static float  c23f(void)  { return bits_to_float(0x4b000000u); }            /* 2^23        */

/* volatile reads keep the compiler from folding or hoisting the adds across fesetround() */
static double vd(double x) { volatile double v = x; return v; }
static float  vf(float x)  { volatile float  v = x; return v; }

/* ---- formulas ------------------------------------------------------------ */
/* (x +d c521) -d c521: double->int64 n/d/u, and "round double to nearest" */
static double f_c521(double x) { return (vd(x) + c521d()) - c521d(); }
/* (x +d c52) -d c52: double->uint64 n/d/u, and "round non-negative double" */
static double f_c52(double x)  { return (vd(x) + c52d()) - c52d(); }
/* double->int64, z (run in mode d or z) */
static double f_z52(double x) {
  double c = c52d();
  return vd(x) >= 0.0 ? (vd(x) + c) - c : c - (c - vd(x));
}
/* low32(x +d c521): double->int32/uint32 n/d/u */
static uint32_t low32_521(double x) { return (uint32_t)double_to_bits(vd(x) + c521d()); }
/* double->int32/uint32, d or z */
static int32_t d2i32_z(double x) {
  if (vd(x) >= 0.0) return (int32_t)low32_521(x);
  return (int32_t)(0u - (uint32_t)double_to_bits(c521d() - vd(x)));
}
/* (x +s c231) -s c231: float->int32 n/d/u, and "round float to nearest" */
static float f_c231(float x) { return (vf(x) + c231f()) - c231f(); }
/* (x +s c23) -s c23: float->uint32 n/d/u, and "round non-negative float" */
static float f_c23(float x)  { return (vf(x) + c23f()) - c23f(); }
/* float->int32, z (run in mode d or z) */
static float f_z23(float x) {
  float c = c23f();
  return vf(x) >= 0.0f ? (vf(x) + c) - c : c - (c - vf(x));
}

static double u64_to_double(uint64_t x) { return bits_to_double(x + 0x4330000000000000ULL) - c52d(); }
static double i64_to_double(int64_t x)  { return bits_to_double((uint64_t)x + 0x4338000000000000ULL) - c521d(); }
static double i32_to_double(int32_t x)  { return i64_to_double(x); }  /* sign-extend (note 4) */
static double u32_to_double(uint32_t x) { return u64_to_double(x); }  /* zero-extend (note 4) */
static float  i32_to_float(int32_t x)   { return bits_to_float((uint32_t)x + 0x4b400000u) - c231f(); }
static float  u32_to_float(uint32_t x)  { return bits_to_float(x + 0x4b000000u) - c23f(); }

/* ---- expected results per rounding mode ---------------------------------- */
enum { RN, RD, RU, RZ };
static const int         fe_mode[]   = { FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO };
static const char *const mode_name[] = { "n", "d", "u", "z" };

typedef struct { double x; long long r[4]; } rnd_case;  /* r[]: n, d, u, z */
static const rnd_case rc[] = {
  /*   x     n   d   u   z */
  {  2.5, { 2,  2,  3,  2}},   /* tie: n goes to even */
  {  3.5, { 4,  3,  4,  3}},
  { -2.5, {-2, -3, -2, -2}},
  { -3.5, {-4, -4, -3, -3}},
  {  2.4, { 2,  2,  3,  2}},
  {  2.6, { 3,  2,  3,  2}},
  { -2.4, {-2, -3, -2, -2}},
  { -2.6, {-3, -3, -2, -2}},
  {  0.5, { 0,  0,  1,  0}},
  { -0.5, { 0, -1,  0,  0}},
  {  7.0, { 7,  7,  7,  7}},
  { -7.0, {-7, -7, -7, -7}},
};

#define FOR_CASES for (size_t i = 0; i < COUNT(rc); i++)
#define MSG "x=%g mode %s: got %lld want %lld"

/* ---- constants ----------------------------------------------------------- */
Test(table_17_2, constants) {
  cr_assert_eq(c521d(), 0x1.8p52);  /* 2^52 + 2^51 */
  cr_assert_eq(c52d(),  0x1p52);
  cr_assert_eq(c231f(), 0x1.8p23f); /* 2^23 + 2^22 */
  cr_assert_eq(c23f(),  0x1p23f);
}

/* ---- integer lives in the mantissa of the sum ------------------------ */
Test(table_17_2, magic_sum_holds_integer_in_mantissa) {
  cr_assert_eq(double_to_bits(5.0  + c521d()), 0x4338000000000000ULL + 5);
  cr_assert_eq(double_to_bits(-5.0 + c521d()), 0x4338000000000000ULL - 5);
  cr_assert_eq(float_to_bits(5.0f  + c231f()), 0x4b400000u + 5);
  cr_assert_eq(float_to_bits(-5.0f + c231f()), 0x4b400000u - 5);

  /* bits(x + c) - bits(c) is the rounded integer */
  cr_assert_eq((int64_t)(double_to_bits(-5.3 + c521d()) - 0x4338000000000000ULL), -5);
  cr_assert_eq((int32_t)(float_to_bits(-5.3f + c231f()) - 0x4b400000u), -5);
}

/* ---- double to int64 ----------------------------------------------------- */
Test(table_17_2, double_to_int64_n_d_u) {
  for (int m = RN; m <= RU; m++) {
    fesetround(fe_mode[m]);
    FOR_CASES {
      long long got = (long long)f_c521(rc[i].x);
      cr_assert_eq(got, rc[i].r[m], MSG, rc[i].x, mode_name[m], got, rc[i].r[m]);
    }
  }
  fesetround(FE_TONEAREST);
}

Test(table_17_2, double_to_int64_z) {            /* works in mode d or z */
  static const int ms[] = { RD, RZ };
  for (size_t k = 0; k < COUNT(ms); k++) {
    fesetround(fe_mode[ms[k]]);
    FOR_CASES {
      long long got = (long long)f_z52(rc[i].x);
      cr_assert_eq(got, rc[i].r[RZ], MSG, rc[i].x, mode_name[ms[k]], got, rc[i].r[RZ]);
    }
  }
  fesetround(FE_TONEAREST);
}

/* ---- double to uint64 (non-negative inputs only) ------------------------- */
Test(table_17_2, double_to_uint64_n_d_u) {
  for (int m = RN; m <= RU; m++) {
    fesetround(fe_mode[m]);
    FOR_CASES {
      if (rc[i].x < 0) continue;
      long long got = (long long)f_c52(rc[i].x);
      cr_assert_eq(got, rc[i].r[m], MSG, rc[i].x, mode_name[m], got, rc[i].r[m]);
    }
  }
  fesetround(FE_TONEAREST);
}

/* ---- double to int32 / uint32: low32 of the sum -------------------------- */
Test(table_17_2, double_to_int32_n_d_u) {
  for (int m = RN; m <= RU; m++) {
    fesetround(fe_mode[m]);
    FOR_CASES {
      long long got = (int32_t)low32_521(rc[i].x);
      cr_assert_eq(got, rc[i].r[m], MSG, rc[i].x, mode_name[m], got, rc[i].r[m]);
    }
  }
  fesetround(FE_TONEAREST);
}

Test(table_17_2, double_to_int32_or_uint32_same_bits) {
  cr_assert_eq(low32_521(-1.0), 0xffffffffu);
  cr_assert_eq(low32_521(4294967295.0), 0xffffffffu);
  cr_assert_eq((int32_t)low32_521(-2147483648.0), INT32_MIN);
  cr_assert_eq(low32_521(2147483647.0), 0x7fffffffu);
}

Test(table_17_2, double_to_int32_z) {
  /* works in mode d or z */
  static const int ms[] = { RD, RZ };
  for (size_t k = 0; k < COUNT(ms); k++) {
    fesetround(fe_mode[ms[k]]);
    FOR_CASES {
      long long got = d2i32_z(rc[i].x);
      cr_assert_eq(got, rc[i].r[RZ], MSG, rc[i].x, mode_name[ms[k]], got, rc[i].r[RZ]);
    }
  }
  fesetround(FE_TONEAREST);
}

/* ---- float to int32 / uint32 --------------------------------------------- */
Test(table_17_2, float_to_int32_n_d_u) {
  for (int m = RN; m <= RU; m++) {
    fesetround(fe_mode[m]);
    FOR_CASES {
      long long got = (long long)f_c231((float)rc[i].x);
      cr_assert_eq(got, rc[i].r[m], MSG, rc[i].x, mode_name[m], got, rc[i].r[m]);
    }
  }
  fesetround(FE_TONEAREST);
}

Test(table_17_2, float_to_int32_z) {
  /* works in mode d or z */
  static const int ms[] = { RD, RZ };
  for (size_t k = 0; k < COUNT(ms); k++) {
    fesetround(fe_mode[ms[k]]);
    FOR_CASES {
      long long got = (long long)f_z23((float)rc[i].x);
      cr_assert_eq(got, rc[i].r[RZ], MSG, rc[i].x, mode_name[ms[k]], got, rc[i].r[RZ]);
    }
  }
  fesetround(FE_TONEAREST);
}

Test(table_17_2, float_to_uint32_n_d_u) {
  for (int m = RN; m <= RU; m++) {
    fesetround(fe_mode[m]);
    FOR_CASES {
      if (rc[i].x < 0) continue;
      long long got = (long long)f_c23((float)rc[i].x);
      cr_assert_eq(got, rc[i].r[m], MSG, rc[i].x, mode_name[m], got, rc[i].r[m]);
    }
  }
  fesetround(FE_TONEAREST);
}

/* ---- round to nearest (result stays floating point) ---------------------- */
Test(table_17_2, round_double_to_nearest) {
  static const double v[] = { -2.5, -0.75, 0.3, 1.5, 2.5, 123456.789, 1000000000000000.5 };
  for (size_t i = 0; i < COUNT(v); i++)
    cr_assert_eq(f_c521(v[i]), nearbyint(v[i]), "x=%.17g", v[i]);
}

Test(table_17_2, round_nonneg_double_to_nearest) {
  static const double v[] = { 0.3, 1.5, 2.5, 123456.789, 4503599627370495.5 /* 2^52 - 0.5 */ };
  for (size_t i = 0; i < COUNT(v); i++)
    cr_assert_eq(f_c52(v[i]), nearbyint(v[i]), "x=%.17g", v[i]);
}

Test(table_17_2, round_float_to_nearest) {
  static const float v[] = { -2.5f, -0.75f, 0.3f, 1.5f, 2.5f, 123456.75f, 4194303.5f /* 2^22 - 0.5 */ };
  for (size_t i = 0; i < COUNT(v); i++)
    cr_assert_eq(f_c231(v[i]), nearbyintf(v[i]), "x=%.9g", (double)v[i]);
}

Test(table_17_2, round_nonneg_float_to_nearest) {
  static const float v[] = { 0.3f, 1.5f, 2.5f, 123456.75f, 8388607.5f /* 2^23 - 0.5 */ };
  for (size_t i = 0; i < COUNT(v); i++)
    cr_assert_eq(f_c23(v[i]), nearbyintf(v[i]), "x=%.9g", (double)v[i]);
}

/* ---- integer to floating point (exact, so rounding mode is irrelevant) --- */
Test(table_17_2, int_to_float_conversions) {
  static const uint64_t u64[] = { 0, 1, 123456789012345ULL, (1ULL << 52) - 1 };
  static const int64_t  i64[] = { 0, 1, -1, 123456789012345LL, -123456789012345LL, 1LL << 51, -(1LL << 51) };
  static const int32_t  i32[] = { 0, 1, -1, 12345, -12345, 1 << 22, -(1 << 22) };
  static const uint32_t u32[] = { 0, 1, 12345, (1u << 23) - 1, 1u << 23 };

  for (int m = RN; m <= RZ; m++) {
    fesetround(fe_mode[m]);
    for (size_t i = 0; i < COUNT(u64); i++) cr_assert_eq(u64_to_double(u64[i]), (double)u64[i]);
    for (size_t i = 0; i < COUNT(i64); i++) cr_assert_eq(i64_to_double(i64[i]), (double)i64[i]);
    for (size_t i = 0; i < COUNT(i32); i++) {
      cr_assert_eq(i32_to_float(i32[i]),  (float)i32[i]);
      cr_assert_eq(i32_to_double(i32[i]), (double)i32[i]);
    }
    for (size_t i = 0; i < COUNT(u32); i++) {
      cr_assert_eq(u32_to_float(u32[i]),  (float)u32[i]);
      cr_assert_eq(u32_to_double(u32[i]), (double)u32[i]);
    }
  }
  fesetround(FE_TONEAREST);
}

Test(table_17_2, uint64_to_double_out_of_range) {
  /* range is 0..2^52-1. 2^52+1 spills into the next binade (ulp 2): result is 2^52+2. */
  uint64_t x = (1ULL << 52) + 1;
  cr_assert_eq(u64_to_double(x), 4503599627370498.0);
  cr_assert_neq(u64_to_double(x), (double)x);
}