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

uint32_t compress(uint32_t x, uint32_t m) {
  uint32_t r, s, b; /* result, shift, mask bit */
  
  r = s = 0;
  do {
    b = m & 1;
    r |= ((x & b) << s);
    s += b;
    x >>= 1; m >>= 1;
  } while (m);
  return r;
}

Test(compress, known_values) {
  cr_assert_eq(compress(0x01234567, 0x000000FF), 0x00000067);
  cr_assert_eq(compress(0x01234567, 0x0000FF00), 0x00000045);
  cr_assert_eq(compress(0x01234567, 0x00FF0000), 0x00000023);
  cr_assert_eq(compress(0x01234567, 0xFF000000), 0x00000001);
}

Test(compress, identity) {
  cr_assert_eq(compress(0x01234567, 0xFFFFFFFF), 0x01234567);
}

Test(compress, zero_mask) {
  cr_assert_eq(compress(0x01234567, 0x00000000), 0x00000000);
}

Test(compress, single_bit_masks) {
  for (int i = 0; i < 32; i++) {
    uint32_t mask = 1U << i;
    cr_assert_eq(compress(0x01234567, mask), (0x01234567 & mask) ? 1 : 0);
  }
}

Test(compress, pop_result_equals_pop_value_and_mask) {
  uint32_t cases[] = { 0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x55555555 };
  uint32_t masks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
    0xAAAAAAAA, 0x55555555, 0xFFFFFFFF };
    
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
      for (size_t j = 0; j < sizeof masks / sizeof masks[0]; j++) {
        uint32_t x = cases[i];
        uint32_t m = masks[j];
        cr_assert_eq(
          __builtin_popcount(compress(x, m)),
          __builtin_popcount(x & m),
          "popcount mismatch for x=0x%08X, m=0x%08X", x, m
        );
      }
    }
  }
  
  uint32_t compress_left(uint32_t x, uint32_t m) {
    uint32_t r, s, b; /* result, shift, mask bit */
  
    r = s = 0;
    do {
      b = m & 1;
      r |= ((x & b) << (31 - s));
      s += b;
      x >>= 1; m >>= 1;
    } while (m);
    return r;
  }
  
  Test(compress_left, packs_into_high_bits) {
    cr_assert_eq(compress_left(0x00000001, 0x00000001), 0x80000000);
    cr_assert_eq(compress_left(0x00000003, 0x00000003), 0xC0000000);
  }
  
  uint32_t expand(uint32_t x, uint32_t m) {
  uint32_t m0, mk, mp, mv, t;
  uint32_t array[5];
  int i;
  
  m0 = m;
  mk = ~m << 1;
  
  for (i = 0; i < 5; i++) {
    mp = mk ^ (mk << 1);
    mp ^= (mp << 2);
    mp ^= (mp << 4);
    mp ^= (mp << 8);
    mp ^= (mp << 16);
    mv = mp & m;
    array[i] = mv;
    m = (m ^ mv) | (mv >> (1 << i));
    mk &= ~mp;
  }
  
  for (i = 4; i >= 0; i--) {
    mv = array[i];
    t = x << (1 << i);
    x = (x & ~mv) | (t & mv);
  }
  
  return x & m0;
}

Test(expand, known_values) {
  cr_assert_eq(expand(0x00000067, 0x000000FF), 0x01234567 & 0x000000FF);
  cr_assert_eq(expand(0x00000045, 0x0000FF00), 0x01234567 & 0x0000FF00);
  cr_assert_eq(expand(0x00000023, 0x00FF0000), 0x01234567 & 0x00FF0000);
  cr_assert_eq(expand(0x00000001, 0xFF000000), 0x01234567 & 0xFF000000);
}

Test(expand, identity) {
  cr_assert_eq(expand(0x01234567, 0xFFFFFFFF), 0x01234567);
}

Test(expand, zero_mask) {
  cr_assert_eq(expand(0x01234567, 0x00000000), 0x00000000);
}

Test(expand, single_bit_mask) {
  cr_assert_eq(expand(0x00000001, 0x00000001), 0x00000001);
  cr_assert_eq(expand(0x00000000, 0x00000001), 0x00000000);
  cr_assert_eq(expand(0x00000001, 0x80000000), 0x80000000);
  cr_assert_eq(expand(0x00000000, 0x80000000), 0x00000000);
}

Test(expand, alternating_masks) {
  /* expand into even bit positions */
  cr_assert_eq(expand(0x0000FFFF, 0xAAAAAAAA), 0xAAAAAAAA);
  /* expand into odd bit positions */
  cr_assert_eq(expand(0x0000FFFF, 0x55555555), 0x55555555);
}

Test(expand, compress_expand_x_mask_equals_x) {
  uint32_t cases[] = { 0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x55555555 };
  uint32_t masks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
                       0xAAAAAAAA, 0x55555555, 0xFFFFFFFF };
  
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    for (size_t j = 0; j < sizeof masks / sizeof masks[0]; j++) {
      uint32_t x = cases[i]; uint32_t m = masks[j];
      uint32_t value = expand(compress(x, m), m);
      cr_assert_eq(
        value, x & m,
        "expand(compress(0x%08X, 0x%08X), 0x%08X) != (0x%08X & 0x%08X)", x, m, m, x, m
      );
    }
  }
}

Test(expand, popcount_of_result_matches_mask) {
  /* expand(x, m) always has at most popcount(m) bits set,
     and exactly popcount(m) bits set when x has enough low bits */
  uint32_t masks[] = { 0x000000FF, 0xAAAAAAAA, 0x55555555, 0x0F0F0F0F };
  for (size_t j = 0; j < sizeof masks / sizeof masks[0]; j++) {
    uint32_t m = masks[j];
    uint32_t result = expand(0xFFFFFFFF, m);
    cr_assert_eq(
      __builtin_popcount(result),
      __builtin_popcount(m),
      "popcount mismatch for mask 0x%08X", m
    );
  }
}

Test(expand, result_always_subset_of_mask) {
  /* expand(x, m) & ~m must always be zero -- no bits outside mask */
  uint32_t cases[] = { 0x00000000, 0xFFFFFFFF, 0x01234567, 0xDEADBEEF };
  uint32_t masks[] = { 0x000000FF, 0xAAAAAAAA, 0x55555555, 0xFF00FF00 };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    for (size_t j = 0; j < sizeof masks / sizeof masks[0]; j++) {
      uint32_t result = expand(cases[i], masks[j]);
      cr_assert_eq(
        result & ~masks[j], 0u,
        "expand(0x%08X, 0x%08X) has bits outside mask", cases[i], masks[j]
      );
    }
  }
}

uint32_t sag(uint32_t x, uint32_t m) {
  return compress_left(x, m) | compress(x, ~m);
}

Test(sag, known_values) {
  cr_assert_eq(sag(0x01234567, 0x000000FF), 0xE6012345);
  cr_assert_eq(sag(0x01234567, 0xFFFFFFFF), 0xE6A2C480); /* all bits selected -> identity */
  cr_assert_eq(sag(0x01234567, 0x00000000), 0x01234567); /* no bits selected -> identity */
}

Test(sag, popcount_invariant) {
  uint32_t cases[] = { 0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x55555555 };
  uint32_t masks[] = { 0x000000FF, 0xAAAAAAAA, 0x55555555, 0x0F0F0F0F };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    for (size_t j = 0; j < sizeof masks / sizeof masks[0]; j++) {
      cr_assert_eq(
        __builtin_popcount(sag(cases[i], masks[j])),
        __builtin_popcount(cases[i]),
        "popcount changed for x=0x%08X m=0x%08X", cases[i], masks[j]
      );
    }
  }
}

Test(sag, selected_bits_in_high_half) {
  /* bits selected by m should appear in the high popcount(m) positions */
  uint32_t x = 0xFFFFFFFF;
  uint32_t m = 0x000000FF; /* 8 bits selected */
  uint32_t result = sag(x, m);
  /* top 8 bits should all be 1 since all selected bits are 1 */
  cr_assert_eq(result >> 24, 0xFF);
}

Test(sag, unselected_bits_in_low_half) {
  uint32_t x = 0xFFFFFFFF;
  uint32_t m = 0xFFFFFF00; /* 24 bits selected, 8 unselected */
  uint32_t result = sag(x, m);
  /* bottom 8 bits should all be 1 since all unselected bits are 1 */
  cr_assert_eq(result & 0xFF, 0xFF);
}

uint32_t permute(uint32_t x, uint32_t p[5]) {
    /* 15-step SAG-based bit permutation (stable binary radix sort) */
    x      = sag(x,    p[0]);
    p[1]   = sag(p[1], p[0]);
    p[2]   = sag(p[2], p[0]);
    p[3]   = sag(p[3], p[0]);
    p[4]   = sag(p[4], p[0]);

    x      = sag(x,    p[1]);
    p[2]   = sag(p[2], p[1]);
    p[3]   = sag(p[3], p[1]);
    p[4]   = sag(p[4], p[1]);

    x      = sag(x,    p[2]);
    p[3]   = sag(p[3], p[2]);
    p[4]   = sag(p[4], p[2]);

    x      = sag(x,    p[3]);
    p[4]   = sag(p[4], p[3]);

    x      = sag(x,    p[4]);
    return x;
}

Test(permute, all_same_p_equals_single_sag) {
  /* when all p[i] are identical, permute == sag(x, p[0]) */
  uint32_t cases[] = { 0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x00000000, 0xFFFFFFFF };
  uint32_t masks[] = { 0xAAAAAAAA, 0x55555555, 0xFF00FF00, 0x0F0F0F0F };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    for (size_t j = 0; j < sizeof masks / sizeof masks[0]; j++) {
      uint32_t x = cases[i];
      uint32_t m = masks[j];
      uint32_t p[5] = { m, m, m, m, m };
      cr_assert_eq(
        permute(x, p),
        sag(x, m),
        "permute(0x%08X, all 0x%08X) != sag result", x, m
      );
    }
  }
}

Test(permute, all_zeros_x) {
  uint32_t p[5] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
  cr_assert_eq(permute(0x00000000, p), 0x00000000);
}

Test(permute, all_ones_x) {
  uint32_t p[5] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
  cr_assert_eq(permute(0xFFFFFFFF, p), 0xFFFFFFFF);
}

Test(permute, popcount_preserved) {
  uint32_t p[5] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
  uint32_t cases[] = { 0x01234567, 0xDEADBEEF, 0xAAAAAAAA, 0x55555555 };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    uint32_t pp[5];
    memcpy(pp, p, sizeof p);
    uint32_t result = permute(cases[i], pp);
    cr_assert_eq(
      __builtin_popcount(result),
      __builtin_popcount(cases[i]),
      "popcount changed for x=0x%08X", cases[i]
    );
  }
}

uint64_t lru_reference(uint64_t m, int i) {
  /* mark line i as most recently used */
  m |=  (uint64_t)0xFF << (8 * i);    /* set all bits in row i */
  m &= ~(0x0101010101010101ULL << i); /* clear column i */
  return m;
}

int lru_find(uint64_t m) {
  /* find least recently used line (first all-zero byte) */
  for (int i = 0; i < 8; i++) {
    if (((m >> (8 * i)) & 0xFF) == 0) return i;
  }
  return -1; /* should never happen */
}

Test(lru, initial_state_all_zero) {
  /* at start, line 0 is LRU (first all-zero byte) */
  uint64_t m = 0x0000000000000000ULL;
  cr_assert_eq(lru_find(m), 0);
}

Test(lru, reference_single_line) {
  uint64_t m = 0x0000000000000000ULL;
  
  /* mark line 0 as most recently used */
  m = lru_reference(m, 0);
  
  /* line 0 is now MRU, line 1 is LRU */
  cr_assert_eq(lru_find(m), 1);
}

Test(lru, reference_sequence) {
  uint64_t m = 0x0000000000000000ULL;
  
  for (int i = 0; i < 8; i++) {
    /* reference lines in order 0,1,2,3,4,5,6,7 */
    m = lru_reference(m, i);
  }
  
  /* line 0 was referenced first so is LRU */
  cr_assert_eq(lru_find(m), 0);
}

Test(lru, reference_sequence_reverse) {
  uint64_t m = 0x0000000000000000ULL;
  
  for (int i = 7; i >= 0; i--) {
    /* reference lines in order 7,6,5,4,3,2,1,0 */
    m = lru_reference(m, i);
  }
  
  /* line 7 was referenced first so is LRU */
  cr_assert_eq(lru_find(m), 7);
}

Test(lru, re_reference_updates_lru) {
  uint64_t m = 0x0000000000000000ULL;
  
  /* reference 0,1,2 line 0 is LRU */
  m = lru_reference(m, 0);
  m = lru_reference(m, 1);
  m = lru_reference(m, 2);
  
  /* line 3 never referenced */
  cr_assert_eq(lru_find(m), 3);
  
  /* now re-reference line 3, line 4 becomes LRU */
  m = lru_reference(m, 3);
  cr_assert_eq(lru_find(m), 4);
}

Test(lru, mru_line_has_full_byte) {
  uint64_t m = 0x0000000000000000ULL;
  
  /* after referencing all 8 lines in order, line 7 (last) is MRU */
  for (int i = 0; i < 8; i++) m = lru_reference(m, i);
  
  /* byte 7 (MRU) should be 0xFF with column 7 cleared = 0x7F */
  uint8_t byte7 = (m >> 56) & 0xFF;
  cr_assert_eq(byte7, 0x7F, "MRU byte should be 0x7F (all set except own column)");
}

Test(lru, lru_line_has_zero_byte) {
  uint64_t m = 0x0000000000000000ULL;
  
  for (int i = 1; i < 8; i++) m = lru_reference(m, i);
  
  /* line 0 never referenced, its byte should be 0 */
  uint8_t byte0 = m & 0xFF;
  cr_assert_eq(byte0, 0x00);
  cr_assert_eq(lru_find(m), 0);
}

Test(lru, reference_order_3_1_2_0) {
  uint64_t m = 0x0000000000000000ULL;
  
  m = lru_reference(m, 3);
  m = lru_reference(m, 1);
  m = lru_reference(m, 2);
  m = lru_reference(m, 0);
  
  /* line 3 was referenced first among these, so is LRU of the four */
  /* lines 4,5,6,7 were never referenced, line 4 is first zero byte */
  cr_assert_eq(lru_find(m), 4);
}