#include <criterion/criterion.h>
#include <stdlib.h>

void mulmns(uint16_t w[], const uint16_t u[], const uint16_t v[], int m, int n) {
  uint32_t k, t, b;
  int i, j;
  
  for (i = 0; i < m + n; i++) { w[i] = 0; }
  for (j = 0; j < n; j++) {
    k = 0;
    for (i = 0; i < m; i++) {
      t = (uint32_t)u[i] * (uint32_t)v[j] + w[i + j] + k;
      w[i + j] = (uint16_t)t;
      k = t >> 16;
    }
    w[j + m] = (uint16_t)k;
  }
  
  int u_negative = (int16_t)u[m - 1] < 0;
  int v_negative = (int16_t)v[n - 1] < 0;

  if (u_negative) {
    b = 0;
    for (j = 0; j < n; j++) {
      t = (uint32_t)w[j + m] - (uint32_t)v[j] - b;
      w[j + m] = (uint16_t)t;
      b = (t >> 16) & 1; /* borrow: did we underflow a 16-bit digit? */
    }
  }
  if (v_negative) {
    b = 0;
    for (i = 0; i < m; i++) {
      t = (uint32_t)w[i + n] - (uint32_t)u[i] - b;
      w[i + n] = (uint16_t)t;
      b = (t >> 16) & 1; /* borrow: did we underflow a 16-bit digit? */
    }
  }
}

Test(mulmns, one_times_one) {
  uint16_t w[4] = {0};
  uint16_t u[2] = {1, 0};
  uint16_t v[2] = {1, 0};
  mulmns(w, u, v, 2, 2);
  cr_assert_eq(w[0], 1); cr_assert_eq(w[1], 0);
  cr_assert_eq(w[2], 0); cr_assert_eq(w[3], 0);
}

Test(mulmns, two_times_three) {
  uint16_t w[4] = {0};
  uint16_t u[2] = {2, 0};
  uint16_t v[2] = {3, 0};
  mulmns(w, u, v, 2, 2);
  cr_assert_eq(w[0], 6);
  cr_assert_eq(w[1], 0);
}

Test(mulmns, large_positive) {
  /* 0x00010000 * 0x00010000 = 0x0000000100000000 */
  uint16_t w[4] = {0};
  uint16_t u[2] = {0x0000, 0x0001};  /* 65536 */
  uint16_t v[2] = {0x0000, 0x0001};  /* 65536 */
  mulmns(w, u, v, 2, 2);
  /* result = 2^32, stored as {0, 0, 1, 0} in little-endian 16-bit digits */
  cr_assert_eq(w[0], 0x0000);
  cr_assert_eq(w[1], 0x0000);
  cr_assert_eq(w[2], 0x0001);
  cr_assert_eq(w[3], 0x0000);
}

Test(mulmns, larger_positive) {
  uint16_t w[8] = {0};
  uint16_t u[4] = {0x0000, 0x0000, 0x0000, 0x0001};
  uint16_t v[4] = {0x0000, 0x0000, 0x0000, 0x0001};
  mulmns(w, u, v, 4, 4);
  /* result = 2^64, stored as {0, 0, 0, 0, 0, 0, 1, 0} in little-endian 16-bit digits */
  cr_assert_eq(w[0], 0x0000);
  cr_assert_eq(w[1], 0x0000);
  cr_assert_eq(w[2], 0x0000);
  cr_assert_eq(w[3], 0x0000);
  cr_assert_eq(w[4], 0x0000);
  cr_assert_eq(w[5], 0x0000);
  cr_assert_eq(w[6], 0x0001);
  cr_assert_eq(w[7], 0x0000);
}

Test(mulmns, negative_times_positive) {
  uint16_t w[4] = {0};
  uint16_t u[2] = {0xFFFF, 0xFFFF};  /* -1 */
  uint16_t v[2] = {0x0001, 0x0000};  /*  1 */
  mulmns(w, u, v, 2, 2);
  cr_assert_eq(w[0], 0xFFFF, "w[0]");
  cr_assert_eq(w[1], 0xFFFF, "w[1]");
  cr_assert_eq(w[2], 0xFFFF, "w[2]");
  cr_assert_eq(w[3], 0xFFFF, "w[3]");
}

Test(mulmns, negative_times_negative) {
  uint16_t w[4] = {0};
  uint16_t u[2] = {0xFFFF, 0xFFFF};  /* -1 */
  uint16_t v[2] = {0xFFFF, 0xFFFF};  /* -1 */
  mulmns(w, u, v, 2, 2);
  cr_assert_eq(w[0], 0x0001, "w[0]");
  cr_assert_eq(w[1], 0x0000, "w[1]");
  cr_assert_eq(w[2], 0x0000, "w[2]");
  cr_assert_eq(w[3], 0x0000, "w[3]");
}

Test(mulmns, negative_two_times_three) {
  uint16_t w[4] = {0};
  uint16_t u[2] = {0xFFFE, 0xFFFF};  /* -2 */
  uint16_t v[2] = {0x0003, 0x0000};  /*  3 */
  mulmns(w, u, v, 2, 2);
  /* -2 * 3 = -6 = 0xFFFFFFFFFFFFFFFA in 4 digits */
  cr_assert_eq(w[0], 0xFFFA, "w[0]");
  cr_assert_eq(w[1], 0xFFFF, "w[1]");
  cr_assert_eq(w[2], 0xFFFF, "w[2]");
  cr_assert_eq(w[3], 0xFFFF, "w[3]");
}

int32_t mulhs(int32_t u, int32_t v) {
  uint32_t u0, v0, w0;
  int32_t u1, v1, w1, w2, t;
  
  u0 = (uint32_t)u & 0xFFFF; u1 = u >> 16;
  v0 = (uint32_t)v & 0xFFFF; v1 = v >> 16;
  w0 = u0 * v0;
  t  = u1 * v0 + (w0 >> 16);
  w1 = t & 0xFFFF; w2 = t >> 16;
  w1 += u0 * v1;
  return u1 * v1 + w2 + (w1 >> 16);
}

Test(mulhs, basic_cases) {
  cr_assert_eq(mulhs(0x00010000, 0x00010000), 0x00000001);
  cr_assert_eq(mulhs(-1, 1), -1);
  cr_assert_eq(mulhs(-1, -1), 0);
}

static int32_t mulhs_ref(int32_t u, int32_t v) {
  return (int32_t)(((int64_t)u * (int64_t)v) >> 32);
}

Test(mulhs, positive_times_positive) {
  cr_assert_eq(mulhs(0x00010000, 0x00010000), 1);
  cr_assert_eq(mulhs(0x00020000, 0x00020000), 4);
  cr_assert_eq(
    mulhs(0x7FFFFFFF, 0x7FFFFFFF),
    mulhs_ref(0x7FFFFFFF, 0x7FFFFFFF)
  );
}

Test(mulhs, negative_times_positive) {
  cr_assert_eq(mulhs(-1,  1), -1);
  cr_assert_eq(mulhs(-2,  3), -1);  /* -6 >> 32 = -1 */
  cr_assert_eq(
    mulhs(0x80000000, 1),
    mulhs_ref(0x80000000, 1)
  );
}

Test(mulhs, negative_times_negative) {
    cr_assert_eq(mulhs(-1, -1),  0);  /* 1 >> 32 = 0 */
    cr_assert_eq(mulhs(-2, -2),  0);  /* 4 >> 32 = 0 */
    cr_assert_eq(
      mulhs(0x80000000, 0x80000000),
      mulhs_ref(0x80000000, 0x80000000)
    );
}

Test(mulhs, zero_cases) {
  cr_assert_eq(mulhs(0, 0x7FFFFFFF), 0);
  cr_assert_eq(mulhs(0x7FFFFFFF, 0), 0);
  cr_assert_eq(mulhs(0, 0), 0);
}

Test(mulhs, agrees_with_reference) {
  int32_t cases[] = {
    0, 1, -1, 2, -2,
    0x7FFFFFFF, 0x80000000,
    0x00010000, 0x0000FFFF,
    0x12345678, 0xDEADBEEF,
  };
  int n = sizeof cases / sizeof cases[0];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      cr_assert_eq(
        mulhs(cases[i], cases[j]),
        mulhs_ref(cases[i], cases[j]),
        "mulhs(0x%08X, 0x%08X)", cases[i], cases[j]
      );
    }
  }
}