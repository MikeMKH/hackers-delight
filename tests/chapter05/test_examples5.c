#include <criterion/criterion.h>
#include <stdlib.h>

Test(counting_bits, divide_and_conquer) {
  uint8_t x = 0b01011000;
  uint8_t e = 3;
  
  x = (x & 0x55) + ((x >> 1) & 0x55);
  x = (x & 0x33) + ((x >> 2) & 0x33);
  x = (x & 0x0F) + ((x >> 4) & 0x0F);
  cr_assert_eq(x, e);
}

Test(counting_bits, turn_off_rightmost_bit_until_zero) {
  uint8_t x = 0b01011000;
  uint8_t e = 3;
  
  int n = 0;
  while (x != 0) {
    x &= (x - 1);
    n++;
  }
  cr_assert_eq(n, e);
}

int pop(uint8_t x) {
  int sum = x;
  while (x != 0) {
    x >>= 1;
    sum -= x;
  }
  return sum;
}

Test(pop, shift_and_subtract) {
  uint8_t x = 0b01011000;
  uint8_t e = 3;
  
  cr_assert_eq(pop(x), e);
}

int pop_diff(uint8_t x, uint8_t y) {
  /* popcount(x) in low nibble, 0..8 */
  x = x - ((x >> 1) & 0x55);
  x = (x & 0x33) + ((x >> 2) & 0x33);
  x = (x + (x >> 4)) & 0x0F;

  /* popcount(~y) = 8 - popcount(y) */
  y = (uint8_t)~y;
  y = y - ((y >> 1) & 0x55);
  y = (y & 0x33) + ((y >> 2) & 0x33);
  y = (y + (y >> 4)) & 0x0F;

  /* popcount(x) + 8 - popcount(y) - 8 = popcount(x) - popcount(y) */
  return (int)(x + y) - 8;
}

Test(pop_diff, x_less_than_y_count) {
  uint8_t x = 0b01011000;
  uint8_t y = 0b10100111;
  int e = -2;
  cr_assert_eq(pop_diff(x, y), e);
}

Test(pop_diff, x_greater_than_y_count) {
  uint8_t x = 0b10101111;
  uint8_t y = 0b01011000;
  int e = 3;
  cr_assert_eq(pop_diff(x, y), e);
}

Test(pop_diff, x_equal_y_count) {
  uint8_t x = 0b01010001;
  uint8_t y = 0b01011000;
  int e = 0;
  cr_assert_eq(pop_diff(x, y), e);
}

Test(pop_diff, all_zero_vs_all_ones) {
  cr_assert_eq(pop_diff(0x00, 0xFF), -8);
}

Test(pop_diff, all_ones_vs_all_zero) {
  cr_assert_eq(pop_diff(0xFF, 0x00), 8);
}

Test(pop_diff, both_zero) {
  cr_assert_eq(pop_diff(0x00, 0x00), 0);
}

int pop_cmp(uint8_t xp, uint8_t yp) {
  uint8_t x, y;
  /* clear bits where both are 1 */
  x = xp & (uint8_t)~yp;
  y = yp & (uint8_t)~xp;
  
  while (1) {
    if (x == 0) return (y != 0) ? -1 : 0;
    if (y == 0) return 1;
    /* clear the rightmost 1 bit in each */
    x &= (uint8_t)(x - 1);
    y &= (uint8_t)(y - 1);
  }
}

Test(pop_cmp, x_less_than_y) {
  uint8_t x = 0b01011000;
  uint8_t y = 0b10100111;
  cr_assert_eq(pop_cmp(x, y), -1);
}

Test(pop_cmp, x_greater_than_y) {
  uint8_t x = 0b10101111;
  uint8_t y = 0b01011000;
  cr_assert_eq(pop_cmp(x, y), 1);
}

Test(pop_cmp, x_equal_y) {
  uint8_t x = 0b01010001;
  uint8_t y = 0b01011000;
  cr_assert_eq(pop_cmp(x, y), 0);
}

Test(pop_cmp, both_zero) {
  uint8_t x = 0b00000000;
  uint8_t y = 0b00000000;
  cr_assert_eq(pop_cmp(x, y), 0);
}

Test(pop_cmp, both_ones) {
  uint8_t x = 0b11111111;
  uint8_t y = 0b11111111;
  cr_assert_eq(pop_cmp(x, y), 0);
}

#define CSA(h, l, a, b, c) \
  {uint8_t u = (a) ^ (b); uint8_t v = (c); \
   h = (a & b) | (u & v); l = u ^ v;}
   
Test(CSA, one_one_zero) {
  uint8_t h, l;
  CSA(h, l, 0b1, 0b1, 0b0);
  cr_assert_eq(h, 0b1);
  cr_assert_eq(l, 0b0);
}

Test(CSA, all_ones) {
  uint8_t h, l;
  CSA(h, l, 0b1, 0b1, 0b1);
  cr_assert_eq(h, 0b1);
  cr_assert_eq(l, 0b1);
}

Test(CSA, all_zeros) {
  uint8_t h, l;
  CSA(h, l, 0b0, 0b0, 0b0);
  cr_assert_eq(h, 0b0);
  cr_assert_eq(l, 0b0);
}

int pop_array(uint8_t A[], int n) {
  int tot, i;
  uint8_t ones, twos;
  
  tot = 0, ones = 0;
  for (i = 0; i <= n - 2; i += 2) {
    CSA(twos, ones, ones, A[i], A[i+1]);
    tot += pop(twos);
  }
  
  tot = 2*tot + pop(ones);
  if (n & 1) tot += pop(A[i]);
  
  return tot;
}

Test(pop_array, basic) {
  uint8_t A[] = {0b10101010, 0b01010101};
  cr_assert_eq(pop_array(A, 2), 8);
}

uint8_t parity(uint8_t x) {
  uint8_t y = x ^ x >> 1;
  y ^= y >> 2;
  y ^= y >> 4;
  return y & 1;
}

Test(parity, examples) {
  cr_assert_eq(parity(0b10101010), 0);
  cr_assert_eq(parity(0b10100000), 0);
  cr_assert_eq(parity(0b10101011), 1);
  cr_assert_eq(parity(0b11100000), 1);
  cr_assert_eq(parity(0b11111111), 0);
  cr_assert_eq(parity(0b00000000), 0);
}

int nlz_32(uint32_t x) {
  if (x == 0) return 32;
  
  int n = 0;
  if (x <= 0x0000FFFF) { n += 16; x <<= 16; }
  if (x <= 0x00FFFFFF) { n += 8; x <<= 8; }
  if (x <= 0x0FFFFFFF) { n += 4; x <<= 4; }
  if (x <= 0x3FFFFFFF) { n += 2; x <<= 2; }
  if (x <= 0x7FFFFFFF) { n += 1; }
  return n;
}

Test(nlz_32, examples) {
  cr_assert_eq(nlz_32(0x00000000), 32);
  cr_assert_eq(nlz_32(0x00000001), 31);
  cr_assert_eq(nlz_32(0x00000002), 30);
  cr_assert_eq(nlz_32(0x00000004), 29);
  cr_assert_eq(nlz_32(0x00000008), 28);
  cr_assert_eq(nlz_32(0x00000010), 27);
  cr_assert_eq(nlz_32(0x00000020), 26);
  cr_assert_eq(nlz_32(0x00000040), 25);
  cr_assert_eq(nlz_32(0x00000080), 24);
  cr_assert_eq(nlz_32(0x00000100), 23);
  cr_assert_eq(nlz_32(0x00000200), 22);
  cr_assert_eq(nlz_32(0x00000400), 21);
  cr_assert_eq(nlz_32(0x00000800), 20);
  cr_assert_eq(nlz_32(0x10000000),  3);
  cr_assert_eq(nlz_32(0x20000000),  2);
  cr_assert_eq(nlz_32(0x40000000),  1);
  cr_assert_eq(nlz_32(0xF0000000),  0);
}

Test(nlz_32, multi_bit_values) {
  cr_assert_eq(nlz_32(0xFFFFFFFF), 0);  /* 11111111111111111111111111111111 */
  cr_assert_eq(nlz_32(0x7FFFFFFF), 1);  /* 01111111111111111111111111111111 */
  cr_assert_eq(nlz_32(0x3FFFFFFF), 2);  /* 00111111111111111111111111111111 */
  cr_assert_eq(nlz_32(0x0FFFFFFF), 4);  /* 00001111111111111111111111111111 */
  cr_assert_eq(nlz_32(0x0000FFFF), 16); /* 00000000000000001111111111111111 */
}

int nlz_8(uint8_t x) {
  if (x == 0) return 8;
  
  int n = 1;
  if ((x >> 4) == 0) { n += 4; x <<= 4; }
  if ((x >> 6) == 0) { n += 2; x <<= 2; }
  n -= (x >> 7);
  return n;
}

Test(nlz_8, examples) {
  cr_assert_eq(nlz_8(0x00), 8);
  cr_assert_eq(nlz_8(0x01), 7);
  cr_assert_eq(nlz_8(0x02), 6);
  cr_assert_eq(nlz_8(0x04), 5);
  cr_assert_eq(nlz_8(0x08), 4);
  cr_assert_eq(nlz_8(0x10), 3);
  cr_assert_eq(nlz_8(0x20), 2);
  cr_assert_eq(nlz_8(0x40), 1);
  cr_assert_eq(nlz_8(0x80), 0);
}

Test(nlz_8, multi_bit_values) {
  cr_assert_eq(nlz_8(0xFF), 0);  /* 11111111 */
  cr_assert_eq(nlz_8(0x7F), 1);  /* 01111111 */
  cr_assert_eq(nlz_8(0x3F), 2);  /* 00111111 */
  cr_assert_eq(nlz_8(0x0F), 4);  /* 00001111 */
  cr_assert_eq(nlz_8(0x07), 5);  /* 00000111 */
  cr_assert_eq(nlz_8(0x03), 6);  /* 00000011 */
  cr_assert_eq(nlz_8(0x05), 5);  /* 00000101 - non-power-of-2 */
  cr_assert_eq(nlz_8(0x0A), 4);  /* 00001010 - non-power-of-2 */
}

int nlz_8_right_propagation(uint8_t x) {
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  return pop((uint8_t)~x);
}

Test(nlz_8_right_propagation, examples) {
  cr_assert_eq(nlz_8_right_propagation(0x00), 8);
  cr_assert_eq(nlz_8_right_propagation(0x01), 7);
  cr_assert_eq(nlz_8_right_propagation(0x02), 6);
  cr_assert_eq(nlz_8_right_propagation(0x04), 5);
  cr_assert_eq(nlz_8_right_propagation(0x08), 4);
  cr_assert_eq(nlz_8_right_propagation(0x10), 3);
  cr_assert_eq(nlz_8_right_propagation(0x20), 2);
  cr_assert_eq(nlz_8_right_propagation(0x40), 1);
  cr_assert_eq(nlz_8_right_propagation(0x80), 0);
}

Test(nlz_8_right_propagation, multi_bit_values) {
  cr_assert_eq(nlz_8_right_propagation(0xFF), 0);  /* 11111111 */
  cr_assert_eq(nlz_8_right_propagation(0x7F), 1);  /* 01111111 */
  cr_assert_eq(nlz_8_right_propagation(0x3F), 2);  /* 00111111 */
  cr_assert_eq(nlz_8_right_propagation(0x0F), 4);  /* 00001111 */
  cr_assert_eq(nlz_8_right_propagation(0x07), 5);  /* 00000111 */
  cr_assert_eq(nlz_8_right_propagation(0x03), 6);  /* 00000011 */
  cr_assert_eq(nlz_8_right_propagation(0x05), 5);  /* 00000101 - non-power-of-2 */
  cr_assert_eq(nlz_8_right_propagation(0x0A), 4);  /* 00001010 - non-power-of-2 */
}

#if defined(__aarch64__)

int nlz_8_float(uint8_t x) {
  if (x == 0) return 8;
  union {
      unsigned asInt[2];
      double asDouble;
  } u;
  u.asDouble = (double)x;
  return 1030 - (int)(u.asInt[1] >> 20);
}

int nlz_8_asm(uint8_t x) {
  /*
    widen to 32-bit
    CLZ counts leading zeros of the 32-bit value,
    subtract 24 to get the 8-bit leading zero count
  */
  if (x == 0) return 8;
  uint32_t w = x;
  uint32_t n;
  __asm__ volatile (
      "clz %w[n], %w[x]"
      : [n] "=r" (n)
      : [x] "r"  (w)
  );
  return (int)(n - 24);
}

Test(nlz_ieee_float, examples) {
  cr_assert_eq(nlz_8_float(0x00), 8);
  cr_assert_eq(nlz_8_float(0x01), 7);
  cr_assert_eq(nlz_8_float(0x02), 6);
  cr_assert_eq(nlz_8_float(0x04), 5);
  cr_assert_eq(nlz_8_float(0x08), 4);
  cr_assert_eq(nlz_8_float(0x10), 3);
  cr_assert_eq(nlz_8_float(0x20), 2);
  cr_assert_eq(nlz_8_float(0x40), 1);
  cr_assert_eq(nlz_8_float(0x80), 0);
}

Test(nlz_arm_clz, examples) {
  cr_assert_eq(nlz_8_asm(0x01), 7);
  cr_assert_eq(nlz_8_asm(0x02), 6);
  cr_assert_eq(nlz_8_asm(0x04), 5);
  cr_assert_eq(nlz_8_asm(0x08), 4);
  cr_assert_eq(nlz_8_asm(0x10), 3);
  cr_assert_eq(nlz_8_asm(0x20), 2);
  cr_assert_eq(nlz_8_asm(0x40), 1);
  cr_assert_eq(nlz_8_asm(0x80), 0);
}

Test(nlz_arm_clz, multi_bit_values) {
  cr_assert_eq(nlz_8_asm(0xFF), 0);
  cr_assert_eq(nlz_8_asm(0x7F), 1);
  cr_assert_eq(nlz_8_asm(0x0F), 4);
  cr_assert_eq(nlz_8_asm(0x05), 5);
  cr_assert_eq(nlz_8_asm(0x0A), 4);
}

/* cross-check: both methods agree on every non-zero value */
Test(nlz_arm_clz, agrees_with_float_method) {
  for (int i = 1; i <= 255; i++) {
    cr_assert_eq(
      nlz_8_asm((uint8_t)i), nlz_8_float((uint8_t)i),
      "mismatch at x=%d", i
    );
  }
}

#else
/* not AArch64 -- skip ARM-specific tests */
Test(nlz_arm_clz, skipped) {
  cr_skip("ARM CLZ tests only run on AArch64");
}
#endif

#include <math.h> /* floor, ceil */

Test(nlz_log_relation, floor_log2) {
  /* floor(log2(x)) = 7 - nlz_8(x) for x != 0 */
  for (int x = 1; x <= 255; x++) {
    int expected = (int)floor(log2((double)x));
    int actual   = 7 - nlz_8((uint8_t)x);
    cr_assert_eq(actual, expected,
      "floor(log2(%d)): got %d, want %d", x, actual, expected);
  }
}

Test(nlz_log_relation, ceil_log2) {
  /* ceil(log2(x)) = 8 - nlz_8(x - 1) for x != 0 */
  for (int x = 1; x <= 255; x++) {
    int expected = (int)ceil(log2((double)x));
    int actual   = 8 - nlz_8((uint8_t)(x - 1));
    cr_assert_eq(actual, expected,
      "ceil(log2(%d)): got %d, want %d", x, actual, expected);
  }
}

int ntz_using_nlz_32(uint32_t x) {
  if (x == 0) return 32;
  return 31 - nlz_32(x & (uint32_t)(-((int32_t)x)));
}

Test(ntz_using_nlz_32, examples) {
  cr_assert_eq(ntz_using_nlz_32(0x00000000), 32);
  cr_assert_eq(ntz_using_nlz_32(0x00000001), 0);
  cr_assert_eq(ntz_using_nlz_32(0x00000002), 1);
  cr_assert_eq(ntz_using_nlz_32(0x00000004), 2);
  cr_assert_eq(ntz_using_nlz_32(0x00000008), 3);
  cr_assert_eq(ntz_using_nlz_32(0x00000010), 4);
  cr_assert_eq(ntz_using_nlz_32(0x00000020), 5);
  cr_assert_eq(ntz_using_nlz_32(0x00000040), 6);
  cr_assert_eq(ntz_using_nlz_32(0x00000080), 7);
  cr_assert_eq(ntz_using_nlz_32(0xFFF00F80), 7);
}

int ntz_using_pop(uint8_t x) {
  if (x == 0) return 8;
  return pop((uint8_t)((x & (uint8_t)(-((int8_t)x))) - 1));
}

Test(ntz_using_pop, examples) {
  cr_assert_eq(ntz_using_pop(0x00), 8);
  cr_assert_eq(ntz_using_pop(0x01), 0);
  cr_assert_eq(ntz_using_pop(0xF1), 0);
  cr_assert_eq(ntz_using_pop(0x02), 1);
  cr_assert_eq(ntz_using_pop(0xF2), 1);
  cr_assert_eq(ntz_using_pop(0x04), 2);
  cr_assert_eq(ntz_using_pop(0xA4), 2);
  cr_assert_eq(ntz_using_pop(0x08), 3);
  cr_assert_eq(ntz_using_pop(0x10), 4);
  cr_assert_eq(ntz_using_pop(0x20), 5);
  cr_assert_eq(ntz_using_pop(0x40), 6);
  cr_assert_eq(ntz_using_pop(0x80), 7);
}

int ntz(uint8_t x) {
  if (x == 0) return 8;
  int n = 1;
  if ((x & 0x0F) == 0) { n += 4; x >>= 4; }
  if ((x & 0x03) == 0) { n += 2; x >>= 2; }
  return n - (x & 1);
}

Test(ntz, examples) {
  cr_assert_eq(ntz(0x00), 8);
  cr_assert_eq(ntz(0x01), 0);
  cr_assert_eq(ntz(0xF1), 0);
  cr_assert_eq(ntz(0x02), 1);
  cr_assert_eq(ntz(0xF2), 1);
  cr_assert_eq(ntz(0x04), 2);
  cr_assert_eq(ntz(0xA4), 2);
  cr_assert_eq(ntz(0x08), 3);
  cr_assert_eq(ntz(0x10), 4);
  cr_assert_eq(ntz(0x20), 5);
  cr_assert_eq(ntz(0x40), 6);
  cr_assert_eq(ntz(0x80), 7);
}

int ntz_search_tree(uint8_t x) {
  if (x & 15) {
    if (x & 3) {
      if (x & 1) return 0;
      else return 1;
    } else if (x & 4) return 2;
      else return 3;
  } else  if (x & 0x30) {
    if (x & 0x10) return 4;
    else return 5;
  } else if (x & 0x40) return 6;
    else if (x) return 7;
    else return 8;
}

Test(ntz_search_tree, examples) {
  cr_assert_eq(ntz_search_tree(0x00), 8);
  cr_assert_eq(ntz_search_tree(0x01), 0);
  cr_assert_eq(ntz_search_tree(0xFF), 0);
  cr_assert_eq(ntz_search_tree(0xF1), 0);
  cr_assert_eq(ntz_search_tree(0x02), 1);
  cr_assert_eq(ntz_search_tree(0xF2), 1);
  cr_assert_eq(ntz_search_tree(0x04), 2);
  cr_assert_eq(ntz_search_tree(0xA4), 2);
  cr_assert_eq(ntz_search_tree(0x08), 3);
  cr_assert_eq(ntz_search_tree(0x10), 4);
  cr_assert_eq(ntz_search_tree(0x20), 5);
  cr_assert_eq(ntz_search_tree(0x40), 6);
  cr_assert_eq(ntz_search_tree(0x80), 7);
}