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