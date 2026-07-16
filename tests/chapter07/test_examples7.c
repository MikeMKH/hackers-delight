#include <criterion/criterion.h>
#include <stdlib.h>

uint32_t rev_interchange_adjacent_bits(uint32_t x) {
  x = ((x & 0x55555555) << 1)  | ((x >>  1) & 0x55555555);
  x = ((x & 0x33333333) << 2)  | ((x >>  2) & 0x33333333);
  x = ((x & 0x0F0F0F0F) << 4)  | ((x >>  4) & 0x0F0F0F0F);
  x = ( x               << 24) | ((x & 0xFF00) << 8)
    | ((x >> 8) & 0xFF00)      |  (x >> 24);
  return x;
}

Test(rev_interchange_adjacent_bits, known_value) {
  cr_assert_eq(rev_interchange_adjacent_bits(0x01234567), 0xE6A2C480);
}

Test(rev_interchange_adjacent_bits, identity_cases) {
  cr_assert_eq(rev_interchange_adjacent_bits(0x00000000), 0x00000000);
  cr_assert_eq(rev_interchange_adjacent_bits(0xFFFFFFFF), 0xFFFFFFFF);
}

Test(rev_interchange_adjacent_bits, single_bits) {
  /* bit 0 (LSB) should move to bit 31 (MSB) and vice versa */
  cr_assert_eq(rev_interchange_adjacent_bits(0x00000001), 0x80000000);
  cr_assert_eq(rev_interchange_adjacent_bits(0x80000000), 0x00000001);
  /* bit 1 <-> bit 30 */
  cr_assert_eq(rev_interchange_adjacent_bits(0x00000002), 0x40000000);
  cr_assert_eq(rev_interchange_adjacent_bits(0x40000000), 0x00000002);
}

Test(rev_interchange_adjacent_bits, involution) {
  /* reversing twice gives back the original */
  uint32_t cases[] = {
    0x00000001, 0x12345678, 0xDEADBEEF,
    0xAAAAAAAA, 0x55555555, 0xFFFFFFFF
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    cr_assert_eq(
      rev_interchange_adjacent_bits(rev_interchange_adjacent_bits(cases[i])), cases[i],
      "involution failed for 0x%08X", cases[i]
    );
  }
}

Test(rev_interchange_adjacent_bits, popcount_preserved) {
  /* reversal never changes the number of set bits */
  uint32_t cases[] = { 0x12345678, 0xDEADBEEF, 0xAAAAAAAA };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    cr_assert_eq(
      __builtin_popcount(rev_interchange_adjacent_bits(cases[i])),
      __builtin_popcount(cases[i]),
      "popcount changed for 0x%08X", cases[i]
    );
  }
}


/* if you don't have __builtin_rotateleft32 */
/*
static uint32_t shlr(uint32_t t, int n) {
  return (t << n) | (t >> (32 - n));
}
*/

uint32_t rev_rotate_shifts(uint32_t x) {
  uint32_t t;
  t = x & 0x00FF00FF; x = __builtin_rotateleft32(t, 16) ^ (t ^ x);
  t = x & 0x0F0F0F0F; x = __builtin_rotateleft32(t,  8) ^ (t ^ x);
  t = x & 0x33333333; x = __builtin_rotateleft32(t,  4) ^ (t ^ x);
  t = x & 0x55555555; x = __builtin_rotateleft32(t,  2) ^ (t ^ x);
  x = __builtin_rotateleft32(x, 1);
  return x;
}

Test(rev_rotate_shifts, agrees_with_interchange) {
  uint32_t cases[] = {
    0x00000000, 0xFFFFFFFF, 0x00000001, 0x80000000,
    0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x55555555,
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    cr_assert_eq(
      rev_rotate_shifts(cases[i]),
      rev_interchange_adjacent_bits(cases[i]),
      "mismatch at 0x%08X", cases[i]
    );
  }
}

uint32_t rev_knuth(uint32_t x) {
  uint32_t t;
  
  x = __builtin_rotateleft32(x, 15);
  t = (x ^ (x >> 10)) & 0x003F801F; x = (t | (t << 10)) ^ x;
  t = (x ^ (x >>  4)) & 0x0E038421; x = (t | (t <<  4)) ^ x;
  t = (x ^ (x >>  2)) & 0x22488842; x = (t | (t <<  2)) ^ x;
  return x;
}

Test(rev_knuth, agrees_with_interchange) {
  uint32_t cases[] = {
    0x00000000, 0xFFFFFFFF, 0x00000001, 0x80000000,
    0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x55555555,
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    cr_assert_eq(
      rev_knuth(cases[i]),
      rev_interchange_adjacent_bits(cases[i]),
      "mismatch at 0x%08X", cases[i]
    );
  }
}

uint32_t increment_rev(uint32_t x) {
  uint32_t m;
  m = 0x80000000;
  x ^= m;
  if ((int)x >= 0) {
    do {
      m >>= 1;
      x ^= m;
    } while (x < m);
  }
  return x;
}

Test(increment_rev, increments_reversed_integers) {
  uint32_t cases[] = {
    0x00000000, 0x00000001, 0x00000002,
    0x00000003, 0x000000FF, 0x0000FFFF,
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    /* increment_rev(rev(x)) == rev(x + 1) */
    uint32_t x        = cases[i];
    uint32_t rev_x    = rev_interchange_adjacent_bits(x);
    uint32_t result   = increment_rev(rev_x);
    uint32_t expected = rev_interchange_adjacent_bits(x + 1);
    cr_assert_eq(result, expected,
      "increment_rev(rev(%u)): got 0x%08X, want 0x%08X", x, result, expected
    );
  }
}