#include <criterion/criterion.h>
#include <stdlib.h>
#include <inttypes.h>

/*
               truncating   modulus    floor
  7÷3           2  r1        2  r1      2  r1
  (-7)÷3       -2  r-1      -3  r2     -3  r2
  7÷(-3)       -2  r1       -2  r1     -3  r-2
  (-7)÷(-3)     2  r-1       3  r2      2  r-1
*/

/* truncating: C built-in behavior */
static int div_trunc(int u, int v) { return u / v; }
static int rem_trunc(int u, int v) { return u % v; }

/* modulus: remainder has same sign as divisor */
static int rem_mod(int u, int v) {
  int r = u % v;
  if (r != 0 && u < 0) r += (v < 0) ? -v : v;
  return r;
}
static int div_mod(int u, int v) {
  int q = u / v;
  int r = u % v;
  if (r != 0 && u < 0) q -= (v < 0) ? -1 : 1;
  return q;
}

/* floor: round toward negative infinity */
static int rem_floor(int u, int v) {
  int r = u % v;
  return r + (((r != 0) & ((u ^ v) < 0)) ? v : 0);
}
static int div_floor(int u, int v) {
  int r = u % v;
  int adjust = ((r != 0) & ((u ^ v) < 0)) ? 1 : 0;
  return u / v - adjust;
}

Test(division, truncating) {
  cr_assert_eq(div_trunc( 7,  3),  2); cr_assert_eq(rem_trunc( 7,  3),  1);
  cr_assert_eq(div_trunc(-7,  3), -2); cr_assert_eq(rem_trunc(-7,  3), -1);
  cr_assert_eq(div_trunc( 7, -3), -2); cr_assert_eq(rem_trunc( 7, -3),  1);
  cr_assert_eq(div_trunc(-7, -3),  2); cr_assert_eq(rem_trunc(-7, -3), -1);
}

Test(division, floor) {
  cr_assert_eq(div_floor( 7,  3),  2); cr_assert_eq(rem_floor( 7,  3),  1);
  cr_assert_eq(div_floor(-7,  3), -3); cr_assert_eq(rem_floor(-7,  3),  2);
  cr_assert_eq(div_floor( 7, -3), -3); cr_assert_eq(rem_floor( 7, -3), -2);
  cr_assert_eq(div_floor(-7, -3),  2); cr_assert_eq(rem_floor(-7, -3), -1);
}

Test(division, modulus) {
  cr_assert_eq(div_mod( 7,  3),  2); cr_assert_eq(rem_mod( 7,  3),  1);
  cr_assert_eq(div_mod(-7,  3), -3); cr_assert_eq(rem_mod(-7,  3),  2);
  cr_assert_eq(div_mod( 7, -3), -2); cr_assert_eq(rem_mod( 7, -3),  1);
  cr_assert_eq(div_mod(-7, -3),  3); cr_assert_eq(rem_mod(-7, -3),  2);
}

Test(division, invariant_q_times_v_plus_r_equals_u) {
  /* for all three: q*v + r == u must always hold */
  int us[] = {7, -7};
  int vs[] = {3, -3};
  for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
    int u = us[i], v = vs[j];
    cr_assert_eq(div_trunc(u,v)*v + rem_trunc(u,v), u, "trunc u=%d v=%d", u, v);
    cr_assert_eq(div_floor(u,v)*v + rem_floor(u,v), u, "floor u=%d v=%d", u, v);
    cr_assert_eq(div_mod(u,v)  *v + rem_mod(u,v),   u, "mod   u=%d v=%d", u, v);
  }
}

int divmnu(unsigned short q[], unsigned short r[],
           const unsigned short u[], const unsigned short v[],
           int m, int n) {
  const unsigned b = 65536; /* Number base (16 bits). */
  unsigned short *un, *vn;  /* Normalized form of u, v. */
  unsigned qhat;            /* Estimated quotient digit. */
  unsigned rhat;            /* A remainder. */
  unsigned p;               /* Product of two digits. */
  int s, i, j, t, k;
  
  if (m < n || n <= 0 || v[n-1] == 0) return 1; /* Return if invalid param. */
  
  if (n == 1) {                    /* Take care of */
    k = 0;                         /* the case of a */
    for (j = m - 1; j >= 0; j--) { /* single-digit */
      q[j] = (k*b + u[j])/v[0];    /* divisor here. */
      k = (k*b + u[j]) - q[j]*v[0];
    }
    if (r != NULL) r[0] = k;  return 0;
  } 
  
  /* Normalize by shifting v left just enough so that
     its high-order bit is on, and shift u left the
     same amount. We may have to append a high-order
     digit on the dividend; we do that unconditionally. */
  
  s = __builtin_clz(v[n-1]) - 16; /* 0 <= s <= 16. */
  vn = (unsigned short *)alloca(2*n);
  for (i = n - 1; i > 0; i--) { vn[i] = (v[i] << s) | (v[i-1] >> (16 - s)); }
  vn[0] = v[0] << s;
  
  un = (unsigned short *)alloca(2*(m + 1));
  un[m] = u[m-1] >> (16 - s);
  for (i = m - 1; i > 0; i--) { un[i] = (u[i] << s) | (u[i-1] >> (16 - s)); }
  un[0] = u[0] << s;
  
  for (j = m - n; j >= 0; j--) { /* Main loop. */
    /* Compute estimate qhat of q[j]. */
    qhat = (un[j+n]*b + un[j+n-1])/vn[n-1];
    p = (unsigned)qhat * (unsigned)vn[i];
    rhat = (un[j+n]*b + un[j+n-1]) - p;
again:
    if (qhat >= b || qhat*vn[n-2] > b*rhat + un[j+n-2]) {
      qhat = qhat - 1;
      rhat = rhat + vn[n-1];
      if (rhat < b) goto again;
    }
    
    /* Multiply and subtract. */
    k = 0;
    for (i = 0; i < n; i++) {
      p = qhat*vn[i];
      t = un[i+j] - k - (p & 0xFFFF);
      un[i+j] = t;
      k = (p >> 16) - (t >> 16);
    }
    t = un[j+n] - k;
    un[j+n] = t;
    q[j] = qhat; /* Store quotient digit. */
    if (t < 0) { /* If we subtracted too */
      q[j] = q[j] - 1; /* much, add back. */
      k = 0;
      for (i = 0; i < n; i++) {
        t = un[i+j] + vn[i] + k;
        un[i+j] = t;  k = t >> 16;
      }
      un[j+n] = un[j+n] + k;
    }
  } /* End j. */
  
  /* If the caller wants the remainder, unnormalize
     it and pass it back. */
  if (r != NULL) {
    for (i = 0; i < n - 1; i++) { r[i] = (un[i] >> s) | (un[i+1] << (16 - s)); }
    r[n-1] = un[n-1] >> s;
  }
  return 0;
}

/*
  helper: build a multiword number from an array of uint32_t limbs
  into uint16_t digits, little-endian
*/
static void to_digits(unsigned short *d, uint32_t val) {
  d[0] = (unsigned short)(val & 0xFFFF);
  d[1] = (unsigned short)(val >> 16);
}

Test(divmnu, invalid_params) {
  unsigned short q[2], r[2], u[2], v[2];
  /* n <= 0 */
  cr_assert_eq(divmnu(q, r, u, v, 2, 0), 1);
  /* m < n */
  cr_assert_eq(divmnu(q, r, u, v, 1, 2), 1);
  /* v[n-1] == 0 */
  v[0] = 0;
  cr_assert_eq(divmnu(q, r, u, v, 2, 1), 1);
}

Test(divmnu, single_digit_divisor) {
  /* 7 / 3 = 2 rem 1 */
  unsigned short u[1] = {7};
  unsigned short v[1] = {3};
  unsigned short q[1], r[1];
  cr_assert_eq(divmnu(q, r, u, v, 1, 1), 0);
  cr_assert_eq(q[0], 2);
  cr_assert_eq(r[0], 1);
}

Test(divmnu, single_digit_exact) {
  /* 12 / 3 = 4 rem 0 */
  unsigned short u[1] = {12};
  unsigned short v[1] = {3};
  unsigned short q[1], r[1];
  cr_assert_eq(divmnu(q, r, u, v, 1, 1), 0);
  cr_assert_eq(q[0], 4);
  cr_assert_eq(r[0], 0);
}

Test(divmnu, two_digit_by_one_digit) {
  /* 0x00010000 / 3 = 21845 rem 1 */
  unsigned short u[2] = {0x0000, 0x0001};  /* 65536 */
  unsigned short v[1] = {3};
  unsigned short q[2], r[1];
  cr_assert_eq(divmnu(q, r, u, v, 2, 1), 0);
  uint32_t quotient = q[0] | ((uint32_t)q[1] << 16);
  cr_assert_eq(quotient, 65536 / 3);
  cr_assert_eq(r[0], 65536 % 3);
}

Test(divmnu, two_digit_by_two_digit) {
  /* 0x00020001 / 0x00010001 = 1 rem 0x00010000 */
  unsigned short u[2], v[2], q[1], r[2];
  to_digits(u, 0x00020001);
  to_digits(v, 0x00010001);
  cr_assert_eq(divmnu(q, r, u, v, 2, 2), 0);
  cr_assert_eq(q[0], 1);
  uint32_t rem = r[0] | ((uint32_t)r[1] << 16);
  cr_assert_eq(rem, 0x00010000);
}

Test(divmnu, large_quotient) {
  /* 0xFFFF0000 / 0xFFFF = 0x10000 rem 0 ... actually pick simpler values */
  /* 0x0002FFFD / 0xFFFF = 3 rem 0 -- 3 * 65535 = 196605 = 0x2FFFD */
  unsigned short u[2], v[1], q[2], r[1];
  to_digits(u, (uint32_t)3 * 0xFFFF);  /* 0x0002FFFD */
  v[0] = 0xFFFF;
  cr_assert_eq(divmnu(q, r, u, v, 2, 1), 0);
  uint32_t qval = q[0] | ((uint32_t)q[1] << 16);
  cr_assert_eq(qval, 3);
  cr_assert_eq(r[0], 0);
}

Test(divmnu, null_remainder) {
  /* passing NULL for r should work */
  unsigned short u[1] = {7};
  unsigned short v[1] = {3};
  unsigned short q[1];
  cr_assert_eq(divmnu(q, NULL, u, v, 1, 1), 0);
  cr_assert_eq(q[0], 2);
}

Test(divmnu, quotient_times_divisor_plus_remainder_equals_dividend) {
  struct { uint32_t u, v; } cases[] = {
    {7,          3},
    {100,        7},
    {0x0000FFFF, 0x0000000F},
    {0x00010000, 3},
    {0x0002FFFD, 0x0000FFFF},
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    unsigned short ud[2], vd[1], q[2], r[2];
    to_digits(ud, cases[i].u);
    vd[0] = (unsigned short)cases[i].v;  /* all v fit in 1 digit here */
    cr_assert_eq(divmnu(q, r, ud, vd, 2, 1), 0);
    uint32_t qval = q[0] | ((uint32_t)q[1] << 16);
    uint32_t rval = r[0];
    cr_assert_eq(
      qval * cases[i].v + rval,
      cases[i].u,
      "invariant failed for u=0x%08X v=0x%08X", cases[i].u, cases[i].v
    );
  }
}

Test(divmnu, agrees_with_builtin) {
  uint32_t us[] = {1, 7, 100, 0xFFFF, 0x10000, 0x0002FFFD};
  uint32_t vs[] = {1, 3, 7,   0xFF,   0xFFFF};  /* all single digit */
  for (size_t i = 0; i < sizeof us / sizeof us[0]; i++) {
    for (size_t j = 0; j < sizeof vs / sizeof vs[0]; j++) {
      if (us[i] < vs[j]) continue;
      unsigned short ud[2], vd[1], q[2], r[1];
      to_digits(ud, us[i]);
      vd[0] = (unsigned short)vs[j];
      
      cr_assert_eq(divmnu(q, r, ud, vd, 2, 1), 0);
      
      uint32_t qval = q[0] | ((uint32_t)q[1] << 16);
      uint32_t rval = r[0];
      
      cr_assert_eq(
        qval,
        us[i] / vs[j],
        "quotient wrong for u=0x%08X v=0x%08X", us[i], vs[j]
      );
      cr_assert_eq(
        rval,
        us[i] % vs[j],
        "remainder wrong for u=0x%08X v=0x%08X", us[i], vs[j]
      );
    }
  }
}

uint32_t divu32(uint64_t n, uint32_t d) {
  if (n < (uint64_t)d) return 0;
  else if (d == 1) return (uint32_t)n;
  else if (d <= 1) return 1;
  else return (uint32_t)(n / d);
}

Test(divu32, basic) {
  cr_assert_eq(divu32(7,  3), 2);
  cr_assert_eq(divu32(7,  7), 1);
  cr_assert_eq(divu32(6,  3), 2);
  cr_assert_eq(divu32(2,  3), 0);  /* n < d */
  cr_assert_eq(divu32(7,  1), 7);  /* d == 1 path */
}

Test(divu32, high_dividend) {
  /* verify by computing expected with uint64_t arithmetic */
  uint64_t n1 = 0x000000FF00000000ULL;
  uint32_t d1 = 0xFFFF;
  cr_assert_eq(divu32(n1, d1), (uint32_t)(n1 / d1));
  
  uint64_t n2 = 0x00000000FFFFFFFEULL;
  uint32_t d2 = 0xFFFF;
  cr_assert_eq(divu32(n2, d2), (uint32_t)(n2 / d2));
  
  /* a case where high bits of n are nonzero and quotient fits in 32 bits */
  uint64_t n3 = 0x0000000200000000ULL;  /* 2 * 2^32 */
  uint32_t d3 = 0x00000003;
  cr_assert_eq(divu32(n3, d3), (uint32_t)(n3 / d3));
  
  /* quotient exactly at 32-bit boundary */
  uint64_t n4 = 0x00000001FFFFFFFEULL;
  uint32_t d4 = 2;
  cr_assert_eq(divu32(n4, d4), (uint32_t)(n4 / d4));
}

Test(divu32, agrees_with_builtin) {
  uint64_t ns[] = {
    1, 7, 100, 0xFFFF, 0x10000, 0x0002FFFD, 0x0000000100000000ULL, 0x00000000FFFFFFFFULL
  };
  uint32_t ds[] = {1, 3, 7, 0xFF, 0xFFFF, 0x80000000};
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++) {
    for (size_t j = 0; j < sizeof ds / sizeof ds[0]; j++) {
      if (ns[i] < ds[j]) continue;
      cr_assert_eq(
        divu32(ns[i], ds[j]),
        (uint32_t)(ns[i] / ds[j]),
        "divu32(0x%016" PRIX64 ", 0x%08X)", ns[i], ds[j]
      );
    }
  }
}

u_int32_t divlu(u_int32_t x, u_int32_t y, u_int32_t z) {
  /* divides (x || y) by z */
  int32_t i;  u_int32_t t;
  for (i = 1; i <= 32; i++) {
    t = (int32_t)x >> 31;      /* All 1’s if x(31) = 1. */
    x = (x << 1) | (y >> 31);  /* Shift x || y left */
    y = y << 1;                /* one bit. */
    if ((x | t) >= z) {  x = x - z;  y = y + 1;  }
  }
  return y; /* remainder is x */
}

Test(divlu, basic) {
  cr_assert_eq(divlu(0, 7, 3), 2);
  cr_assert_eq(divlu(0, 7, 7), 1);
  cr_assert_eq(divlu(0, 6, 3), 2);
  cr_assert_eq(divlu(0, 2, 3), 0);  /* n < d */
  cr_assert_eq(divlu(0, 7, 1), 7);  /* d == 1 path */
  
  /* test with nonzero high word */
  cr_assert_eq(divlu(1, 0, 3), (uint32_t)(((uint64_t)1 << 32) / (uint64_t)3));
}

Test(divlu, agrees_with_builtin) {
  struct { uint32_t x, y, z; } cases[] = {
    {0, 7,          3},
    {0, 100,        7},
    {0, 0xFFFFFFFF, 3},
    {0, 0xFFFFFFFF, 0xFFFF},
    {0, 0xFFFFFFFE, 0xFFFF},
    {1, 0,          3},
    {1, 0,          0xFFFFFFFF},
    {2, 0,          3},
    {0xFF, 0,       0xFFFF},
    {0xFFFF, 0,     0xFFFF},
    {1, 1,          3},
    {0, 0xFFFFFFFF, 0x80000000},
    {1, 0xFFFFFFFF, 0xFFFFFFFF},
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    uint32_t x = cases[i].x, y = cases[i].y, z = cases[i].z;
    uint64_t n = ((uint64_t)x << 32) | y;

    if (z == 0) continue;
    if (n < (uint64_t)z) continue;
    if (n / z > 0xFFFFFFFFULL) continue;  /* quotient overflows 32 bits */

    uint32_t expected = (uint32_t)(n / z);
    uint32_t got = divlu(x, y, z);
    cr_assert_eq(
      got, expected,
      "divlu(0x%08X, 0x%08X, 0x%08X): got 0x%08X want 0x%08X", x, y, z, got, expected
    );
  }
}


Test(divlu, remainder_check) {
  /*
    remainder is left in x after the call we can verify with
    quotient * z + remainder == n
  */
  struct { uint32_t x, y, z; } cases[] = {
    {0, 7,  3},
    {0, 7,  2},
    {1, 0,  3},
    {0xFF, 0x12345678, 0xFFFF},
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    uint32_t x = cases[i].x, y = cases[i].y, z = cases[i].z;
    uint64_t n = ((uint64_t)x << 32) | y;
    
    if (n < z) continue;
    
    uint32_t q = divlu(x, y, z);
    uint64_t rem = n % z;
    cr_assert_eq(
      (uint64_t)q * z + rem, n,
        "invariant failed for (0x%08X||0x%08X)/0x%08X", x, y, z
    );
  }
}

Test(divlu, boundary_values) {
  /* divisor = 1: quotient = full dividend */
  cr_assert_eq(divlu(0, 0xFFFFFFFF, 1), 0xFFFFFFFF);
  cr_assert_eq(divlu(0, 0,          1), 0);
  
  /* dividend just below divisor: quotient = 0 */
  cr_assert_eq(divlu(0, 2, 3), 0);
  
  /* dividend == divisor: quotient = 1 */
  cr_assert_eq(divlu(0, 0xFFFFFFFF, 0xFFFFFFFF), 1);
  
  /* power of 2 divisor */
  cr_assert_eq(divlu(0, 0x80000000, 2), 0x40000000);
  cr_assert_eq(divlu(0, 256, 16), 16);
  
  /* quotient exactly fills 32 bits */
  cr_assert_eq(
    divlu(1, 0xFFFFFFFE, 2),
    (uint32_t)(((uint64_t)1 << 32 | 0xFFFFFFFE) / 2)
  );
}

Test(divlu, high_word_patterns) {
  uint32_t highs[] = { 0, 1, 2, 0xFF, 0xFFFF, 0x7FFFFFFF };
  uint32_t divs[]  = { 3, 7, 0xFFFF, 0x80000001, 0xFFFFFFFF };
  for (size_t i = 0; i < sizeof highs / sizeof highs[0]; i++) {
    for (size_t j = 0; j < sizeof divs / sizeof divs[0]; j++) {
      uint32_t x = highs[i], y = 0x12345678, z = divs[j];
      uint64_t n = ((uint64_t)x << 32) | y;
      
      if (z == 0) continue;
      if (n < (uint64_t)z) continue;
      if (n / z > 0xFFFFFFFFULL) continue;  /* quotient overflows */
      
      uint32_t expected = (uint32_t)(n / z);
      cr_assert_eq(
        divlu(x, y, z),
        expected,
        "divlu(0x%08X, 0x%08X, 0x%08X)", x, y, z
      );
    }
  }
}