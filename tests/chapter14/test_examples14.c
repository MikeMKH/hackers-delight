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
  printf("\n\n--start parity trace--\n");
  parity_xor_tree_print_trace(0b10110100);
  printf("--end parity trace--\n\n");
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

typedef struct {
  uint32_t quotient;
  uint32_t remainder;
} gf2_divmod_result;

/*
  Degree of a GF(2) polynomial packed into an unsigned int, one bit
  per coefficient (bit i is the coefficient of x^i). Returns -1 for
  the zero polynomial; documented domain boundary, not an error
  path: callers that can't reach p == 0 don't need to check for it,
  callers that can should check gf2_degree's return before using it
  as a shift amount.
*/
int gf2_degree(uint32_t p) {
  if (p == 0) return -1;
  return 31 - __builtin_clz(p);
}

/*
  GF(2) polynomial long division: dividend / divisor, same algorithm
  as ordinary long division with subtraction replaced by XOR (so
  there's never a borrow). divisor must not be the zero polynomial; 
  that precondition is on the caller, the same way dividing by the
  integer 0 is on the caller elsewhere in C.
*/
gf2_divmod_result gf2_divide(uint32_t dividend, uint32_t divisor) {
  int div_deg = gf2_degree(divisor);
  uint32_t remainder = dividend;
  uint32_t quotient = 0;
  int rem_deg = gf2_degree(remainder);

  /*
    Same shape as the by-hand walkthrough: while the remainder's
    leading term is at least as high-degree as the divisor's, shift
    the divisor up to align leading terms and XOR it in -- no
    subtraction, no borrow, ever.
  */
  while (rem_deg >= div_deg && rem_deg >= 0) {
    int shift = rem_deg - div_deg;
    remainder ^= divisor << shift;
    quotient |= (uint32_t)1 << shift;
    rem_deg = gf2_degree(remainder);
  }

  gf2_divmod_result result;
  result.quotient = quotient;
  result.remainder = remainder;
  return result;
}

void gf2_divide_print_trace(uint32_t dividend, uint32_t divisor) {
  int div_deg = gf2_degree(divisor);
  uint32_t remainder = dividend;
  uint32_t quotient = 0;
  int rem_deg = gf2_degree(remainder);

  printf("dividend   : 0x%08x (degree %d)\n", dividend, rem_deg);
  printf("divisor    : 0x%08x (degree %d)\n", divisor, div_deg);

  while (rem_deg >= div_deg && rem_deg >= 0) {
    int shift = rem_deg - div_deg;
    uint32_t shifted_divisor = divisor << shift;
    printf("  bring in x^%-2d term -> xor 0x%08x\n", shift, shifted_divisor);
    remainder ^= shifted_divisor;
    quotient |= (uint32_t)1 << shift;
    rem_deg = gf2_degree(remainder);
    printf("  remainder now      : 0x%08x (degree %d)\n", remainder, rem_deg);
  }

  printf("quotient   : 0x%08x\n", quotient);
  printf("remainder  : 0x%08x\n", remainder);
}

/*
  Carry-less (GF(2)) multiplication of two <=32-bit polynomials. Used
  here only as an independent cross-check for gf2_divide; not the
  computation under test, a second one to catch it being wrong.
  Widened to 64 bits since the product can need up to 63 bits.
*/
uint64_t gf2_multiply(uint32_t a, uint32_t b) {
  uint64_t result = 0;
  for (int i = 0; i < 32; i++) {
    if (b & ((uint32_t)1 << i)) {
      result ^= (uint64_t)a << i;
    }
  }
  return result;
}

Test(gf2_div, degree_known_values) {
  cr_assert_eq(gf2_degree(0u), -1);
  cr_assert_eq(gf2_degree(1u), 0);
  cr_assert_eq(gf2_degree(0b1011u), 3);
  cr_assert_eq(gf2_degree(0b11100110u), 7);
}

/* book's example: x^7+x^6+x^5+x^2+x divided by x^3+x+1 equals x^4+x^3+1 remainder x^2+1 */
Test(gf2_div, book_example) {
  gf2_divmod_result r = gf2_divide(0b11100110u, 0b1011u);
  cr_assert_eq(r.quotient, 0b11001u, "got quotient 0x%x", r.quotient);
  cr_assert_eq(r.remainder, 0b101u, "got remainder 0x%x", r.remainder);
}

/* smaller example: x^3+x+1 divided by x+1 equals x^2+x remainder 1 */
Test(gf2_div, small_example) {
  gf2_divmod_result r = gf2_divide(0b1011u, 0b11u);
  cr_assert_eq(r.quotient, 0b110u, "got quotient 0x%x", r.quotient);
  cr_assert_eq(r.remainder, 0b1u, "got remainder 0x%x", r.remainder);
}

Test(gf2_div, divides_evenly_when_remainder_is_zero) {
  /* (x+1) * (x+1) = x^2+1 */
  gf2_divmod_result r = gf2_divide(0b101u, 0b11u);
  cr_assert_eq(r.quotient, 0b11u, "got quotient 0x%x", r.quotient);
  cr_assert_eq(r.remainder, 0u, "got remainder 0x%x", r.remainder);
}

/*
  Cross-check against an independently-computed gf2_multiply, over an
  exhaustive sweep of small dividends/divisors. For every case, two
  things must hold: (1) quotient * divisor XOR remainder reconstructs
  the dividend, and (2) the remainder's degree is strictly less than
  the divisor's; the stopping condition gf2_divide relies on.
*/
Test(gf2_div, multiply_back_matches_exhaustive) {
  for (uint32_t dividend = 0; dividend <= 0xFF; dividend++) {
    for (uint32_t divisor = 1; divisor <= 0xF; divisor++) {
      gf2_divmod_result r = gf2_divide(dividend, divisor);
      uint64_t reconstructed = gf2_multiply(r.quotient, divisor) ^ r.remainder;
      cr_assert_eq(
        reconstructed, (uint64_t)dividend,
        "dividend=0x%x divisor=0x%x quotient=0x%x remainder=0x%x reconstructed=0x%llx",
        dividend, divisor, r.quotient, r.remainder,
        (unsigned long long)reconstructed
      );
      cr_assert_lt(
        gf2_degree(r.remainder), gf2_degree(divisor),
        "dividend=0x%x divisor=0x%x remainder=0x%x had degree >= divisor's",
        dividend, divisor, r.remainder
      );
    }
  }
}

/* No assertions use to see the long division unfold */
Test(gf2_div, trace) {
  printf("\n\n--start GF(2) long division trace--\n");
  gf2_divide_print_trace(0b11100110u, 0b1011u);
  printf("--end GF(2) long division trace--\n\n");
  /*
   dividend   : 0x000000e6 (degree 7)
   divisor    : 0x0000000b (degree 3)
     bring in x^4  term -> xor 0x000000b0
     remainder now      : 0x00000056 (degree 6)
     bring in x^3  term -> xor 0x00000058
     remainder now      : 0x0000000e (degree 3)
     bring in x^0  term -> xor 0x0000000b
     remainder now      : 0x00000005 (degree 2)
   quotient   : 0x00000019
   remainder  : 0x00000005
  */
  cr_assert(1, "trace-only test");
}