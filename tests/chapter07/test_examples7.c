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

uint32_t outer_shuffle(uint32_t x) {
  x = ((x & 0x0000FF00) <<  8) | ((x >>  8) & 0x0000FF00) | (x & 0xFF0000FF);
  x = ((x & 0x00F000F0) <<  4) | ((x >>  4) & 0x00F000F0) | (x & 0xF00FF00F);
  x = ((x & 0x0C0C0C0C) <<  2) | ((x >>  2) & 0x0C0C0C0C) | (x & 0xC3C3C3C3);
  x = ((x & 0x22222222) <<  1) | ((x >>  1) & 0x22222222) | (x & 0x99999999);
  return x;
}

Test(outer_shuffle, half_ones) {
  /* left half all 1s, right half all 0s -> alternating 10101010... */
  cr_assert_eq(outer_shuffle(0xFFFF0000), 0xAAAAAAAA);
  /* left half all 0s, right half all 1s -> alternating 01010101... */
  cr_assert_eq(outer_shuffle(0x0000FFFF), 0x55555555);
}

Test(outer_shuffle, identity_cases) {
  cr_assert_eq(outer_shuffle(0x00000000), 0x00000000);
  cr_assert_eq(outer_shuffle(0xFFFFFFFF), 0xFFFFFFFF);
}

Test(outer_shuffle, cycle_test) {
  /*
    0xFFFF0000 → 0xAAAAAAAA → 0xCCCCCCCC → 0xF0F0F0F0 → 0xFF00FF00 → 0xFFFF0000
  */
  cr_assert_eq(outer_shuffle(0xFFFF0000), 0xAAAAAAAA);
  cr_assert_eq(outer_shuffle(0xAAAAAAAA), 0xCCCCCCCC);
  cr_assert_eq(outer_shuffle(0xCCCCCCCC), 0xF0F0F0F0);
  cr_assert_eq(outer_shuffle(0xF0F0F0F0), 0xFF00FF00);
  cr_assert_eq(outer_shuffle(0xFF00FF00), 0xFFFF0000);
}

Test(outer_shuffle, single_bits) {
  /* bit 31 (MSB of left half) -> bit 31 */
  cr_assert_eq(outer_shuffle(0x80000000), 0x80000000);
  /* bit 16 (LSB of left half) -> bit 1 */
  cr_assert_eq(outer_shuffle(0x00010000), 0x00000002);
  /* bit 15 (MSB of right half) -> bit 30 */
  cr_assert_eq(outer_shuffle(0x00008000), 0x40000000);
  /* bit 0 (LSB of right half) -> bit 0 */
  cr_assert_eq(outer_shuffle(0x00000001), 0x00000001);
}

Test(outer_shuffle, cycle_length_5) {
  uint32_t x = 0x01234567;
  uint32_t orig = x;
  for (int i = 0; i < 5; i++) x = outer_shuffle(x);
  cr_assert_eq(x, orig, "shuffle cycle length is not 5");
}

Test(outer_shuffle, multiplicative_order_of_2_mod_31) {
  /* 2^5 ≡ 1 (mod 31) means shuffle has cycle length 5 */
  uint32_t power = 1;
  for (int i = 0; i < 5; i++) power = (power * 2) % 31;
  cr_assert_eq(power, 1, "2^5 mod 31 should be 1");
  
  /* a smaller power does not work */
  power = 1;
  for (int i = 0; i < 4; i++) {
    power = (power * 2) % 31;
    cr_assert_neq(power, 1, "2^%d mod 31 should not be 1", i + 1);
  }
}

Test(outer_shuffle, cycle_traces_powers_of_2_mod_31) {
  /* each shuffle step is multiplication by 2 mod 31 in bit-position space */
  /* 0xFFFF0000 has bits 16-31 set, after shuffle bit i moves to 2i mod 31 */
  uint32_t x = 0xFFFF0000;
  uint32_t cycle[] = {
    0xFFFF0000,  /* bits at positions: 16,17,...,31         */
    0xAAAAAAAA,  /* bits at positions: 32mod31=1,2,4,6,...  */
    0xCCCCCCCC,
    0xF0F0F0F0,
    0xFF00FF00,
  };
  for (int i = 0; i < 5; i++) {
    cr_assert_eq(x, cycle[i], "step %d: got 0x%08X", i, x);
    x = outer_shuffle(x);
  }
  cr_assert_eq(x, cycle[0], "should return to start after 5 shuffles");
}

uint32_t inner_shuffle(uint32_t x) {
  x = ((x & 0x22222222) <<  1) | ((x >>  1) & 0x22222222) | (x & 0x99999999);
  x = ((x & 0x0C0C0C0C) <<  2) | ((x >>  2) & 0x0C0C0C0C) | (x & 0xC3C3C3C3);
  x = ((x & 0x00F000F0) <<  4) | ((x >>  4) & 0x00F000F0) | (x & 0xF00FF00F);
  x = ((x & 0x0000FF00) <<  8) | ((x >>  8) & 0x0000FF00) | (x & 0xFF0000FF);
  return x;
}

Test(inner_shuffle, consistent_with_outer_inverse) {
  /* inner(outer(x)) = x means inner undoes what outer did */
  cr_assert_eq(inner_shuffle(outer_shuffle(0x0000FFFF)), 0x0000FFFF);
  cr_assert_eq(inner_shuffle(outer_shuffle(0xFFFF0000)), 0xFFFF0000);
}

Test(inner_shuffle, is_inverse_of_outer) {
  uint32_t cases[] = {
    0x00000000, 0xFFFFFFFF, 0xFFFF0000, 0x0000FFFF,
    0xAAAAAAAA, 0x55555555, 0x01234567, 0xDEADBEEF,
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    cr_assert_eq(
      inner_shuffle(outer_shuffle(cases[i])), cases[i],
      "inner(outer(0x%08X)) != identity", cases[i]
    );
    cr_assert_eq(outer_shuffle(
      inner_shuffle(cases[i])), cases[i],
      "outer(inner(0x%08X)) != identity", cases[i]
    );
  }
}

Test(inner_shuffle, half_ones) {
  cr_assert_eq(outer_shuffle(0x00FF00FF), 0x0000FFFF);
  cr_assert_eq(outer_shuffle(0xFF00FF00), 0xFFFF0000);
}

Test(inner_shuffle, cycle_length_5) {
  uint32_t cases[] = { 0x01234567, 0xDEADBEEF, 0xFFFF0000 };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    uint32_t x = cases[i];
    for (int j = 0; j < 5; j++) x = inner_shuffle(x);
    cr_assert_eq(x, cases[i], "5x inner shuffle failed for 0x%08X", cases[i]);
  }
}