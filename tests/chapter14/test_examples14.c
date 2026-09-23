#include <criterion/criterion.h>
#include <stdint.h>

/*
  Computes the parity of a byte using an XOR tree.
  Returns 1 if the number of 1-bits in b is odd, 0 otherwise.
*/

#include <stdio.h>
 
#if defined(__aarch64__)
#include <arm_neon.h>
#endif
 
uint8_t parity_xor_tree(uint8_t b) {
  uint8_t x = b;
  x = (uint8_t)(x ^ (x >> 4));
  x = (uint8_t)(x ^ (x >> 2));
  x = (uint8_t)(x ^ (x >> 1));
  return x & 1u;
}
 
void parity_xor_tree_print_trace(uint8_t b) {
  uint8_t x = b;
  printf("start      : 0x%02x\n", x);
  x = (uint8_t)(x ^ (x >> 4));
  printf("after >>4  : 0x%02x\n", x);
  x = (uint8_t)(x ^ (x >> 2));
  printf("after >>2  : 0x%02x\n", x);
  x = (uint8_t)(x ^ (x >> 1));
  printf("after >>1  : 0x%02x\n", x);
  printf("parity bit : %u\n", x & 1u);
}
 
#if defined(__aarch64__)
uint8_t parity_neon(uint8_t b) {
  /*
    CNT does a native per-byte population count, therefore no PSHUFB-style
    nibble lookup table needed the way x86 SIMD popcount tricks require.
    Loading a single byte into an 8x8 vector is overkill for one value;
    this exists purely as an example.
 
    vdup_n_u8 broadcasts b into all 8 lanes, so vcnt_u8 computes the
    same popcount of b in every lane. Read back just one lane (they're
    all identical). DO NOT vaddv_u8-reduce across lanes, that would
    sum 8 copies of the same count instead of reporting it once.
  */
  uint8x8_t v = vdup_n_u8(b);
  uint8x8_t counted = vcnt_u8(v);
  uint8_t popcount = vget_lane_u8(counted, 0);
  return popcount & 1u;
}
#endif

Test(parity, matches_builtin_parity_exhaustive) {
  for (int i = 0; i <= 0xFF; i++) {
    uint8_t b = (uint8_t)i;
    uint8_t got = parity_xor_tree(b);
    uint8_t want = (uint8_t)__builtin_parity((unsigned)b);
    cr_assert_eq(got, want, "b=0x%02x got=%u want=%u", b, got, want);
  }
}
 
Test(parity, known_values) {
  cr_assert_eq(parity_xor_tree(0b00000000), 0);
  cr_assert_eq(parity_xor_tree(0b00000001), 1);
  cr_assert_eq(parity_xor_tree(0b10000001), 0);
  cr_assert_eq(parity_xor_tree(0b10101010), 0);
  cr_assert_eq(parity_xor_tree(0b11101010), 1);
}
 
/* No assertions used to see the tree fold step by step */
Test(parity, trace) {
  parity_xor_tree_print_trace(0b10110100);
  /*
  start      : 0xb4
  after >>4  : 0xbf
  after >>2  : 0x90
  after >>1  : 0xd8
  parity bit : 0
  */
  cr_assert(1, "trace-only test");
}

 
#if defined(__aarch64__)
Test(parity, neon_matches_xor_tree_exhaustive) {
  for (int i = 0; i <= 0xFF; i++) {
    uint8_t b = (uint8_t)i;
    cr_assert_eq(
      parity_neon(b), parity_xor_tree(b),
      "b=0x%02x xor_tree=%u neon=%u",
      b, parity_xor_tree(b), parity_neon(b)
    );
  }
}
#endif