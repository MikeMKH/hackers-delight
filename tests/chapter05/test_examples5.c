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