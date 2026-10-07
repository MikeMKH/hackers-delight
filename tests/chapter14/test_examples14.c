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

uint32_t crc32(uint8_t *message) {
  int32_t i, j;
  uint32_t byte, crc, mask;
  i = 0;  crc = 0xFFFFFFFF;
  while (message[i] != 0) {
    byte = message[i];  /* Get next byte. */
    crc ^= byte;

    for (j = 7; j >= 0; j--) {
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }

    i++;
  }
  return ~crc;
}

uint32_t crc32_reference(uint8_t *message) {
  uint32_t crc = 0xFFFFFFFF;
  for (int i = 0; message[i] != 0; i++) {
    crc ^= message[i];
    for (int bit = 0; bit < 8; bit++) {
      if (crc & 1u) {
        crc = (crc >> 1) ^ 0xEDB88320u;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return ~crc;
}

void crc32_print_byte_trace(uint32_t crc_in, uint8_t byte) {
  uint32_t crc = crc_in ^ byte;
  printf("crc after xor byte 0x%02x : 0x%08x\n", byte, crc);
  for (int j = 7; j >= 0; j--) {
    uint32_t mask = -(crc & 1u);
    printf("  bit shifted out=%u mask=0x%08x -> ", crc & 1u, mask);
    crc = (crc >> 1) ^ (0xEDB88320u & mask);
    printf("crc=0x%08x\n", crc);
  }
}

#include <string.h>

/* Published CRC-32 check value */
Test(crc32, check_value_123456789) {
  cr_assert_eq(crc32((uint8_t *)"123456789"), 0xCBF43926u);
}

/*
  Empty message: crc starts and ends at the init value, complemented.
  Demonstrates exactly why init/xorout exist -- without them this
  would be 0 regardless of message content for any all-zero input.
*/
Test(crc32, empty_message_is_zero) {
  cr_assert_eq(crc32((uint8_t *)""), 0u);
}

Test(crc32, known_values) {
  cr_assert_eq(crc32((uint8_t *)"a"), 0xE8B7BE43u);
  cr_assert_eq(crc32((uint8_t *)"The quick brown fox jumps over the lazy dog"), 0x414FA339u);
}

/*
  Cross-check against crc32_reference known good implementation
   over every possible single byte, rather than relying only on the
  hand-checked strings above.
*/
Test(crc32, matches_reference_every_single_byte) {
  for (unsigned b = 1; b <= 0xFF; b++) {
    uint8_t message[2] = { (uint8_t)b, 0 };
    uint32_t got  = crc32(message);
    uint32_t want = crc32_reference(message);
    cr_assert_eq(got, want, "byte=0x%02x got=0x%08x want=0x%08x", b, got, want);
  }
}

Test(crc32, matches_reference_representative_strings) {
  const char *messages[] = {
    "a", "ab", "abc", "123456789",
    "The quick brown fox jumps over the lazy dog",
    "\x01\x02\x03\x04\x05",
  };
  for (size_t k = 0; k < sizeof(messages) / sizeof(messages[0]); k++) {
    uint8_t *m = (uint8_t *)messages[k];
    cr_assert_eq(crc32(m), crc32_reference(m), "message index %zu mismatched", k);
  }
}

/*
  Single-bit-error detection: CRC-32 is guaranteed to detect any
  single-bit corruption. Flip each bit of a fixed message one at a
  time and confirm every single flip changes the checksum -- an
  exhaustive check over every bit position of this one message, not
  a sample.
*/
Test(crc32, single_bit_flip_always_changes_crc) {
  uint8_t message[] = "test message";
  size_t len = strlen((char *)message);
  uint32_t original = crc32(message);

  for (size_t byte_idx = 0; byte_idx < len; byte_idx++) {
    for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
      uint8_t flipped[32];
      memcpy(flipped, message, sizeof(message)); /* include the NUL */
      flipped[byte_idx] ^= (uint8_t)(1u << bit_idx);
      uint32_t flipped_crc = crc32(flipped);
      cr_assert_neq(
        flipped_crc, original,
        "byte %zu bit %d: flip did not change the CRC",
        byte_idx, bit_idx
      );
    }
  }
}

/* No assertions used to watch the mask trick unfold bit by bit for one byte */
Test(crc32, trace) {
  printf("\n\n--start CRC-32 byte trace--\n");
  crc32_print_byte_trace(0xFFFFFFFFu, 'a');
  printf("--end CRC-32 byte trace--\n\n");
/*
crc after xor byte 0x61 : 0xffffff9e
  bit shifted out=0 mask=0x00000000 -> crc=0x7fffffcf
  bit shifted out=1 mask=0xffffffff -> crc=0xd2477cc7
  bit shifted out=1 mask=0xffffffff -> crc=0x849b3d43
  bit shifted out=1 mask=0xffffffff -> crc=0xaff51d81
  bit shifted out=1 mask=0xffffffff -> crc=0xba420de0
  bit shifted out=0 mask=0x00000000 -> crc=0x5d2106f0
  bit shifted out=0 mask=0x00000000 -> crc=0x2e908378
  bit shifted out=0 mask=0x00000000 -> crc=0x174841bc
*/
  cr_assert(1, "trace-only test");
}

typedef struct {
  int x2;
  int x1;
  int x0;
} crc_circuit_state;

crc_circuit_state crc_circuit_init(void) {
  crc_circuit_state s;
  s.x2 = 0;
  s.x1 = 0;
  s.x0 = 0;
  return s;
}

/* One clock cycle: feed in one message bit (0 or 1), return the new state. */
crc_circuit_state crc_circuit_step(crc_circuit_state state, int message_bit) {
  /*
    Directly transcribed from Figure 14-3's wiring:
 
         Message
          Input
            |
            v
        .-----.    +------+    +------+    .-----.    +------+
        | (+) |<---|  x^2 |<---|  x^1 |<---| (+) |<---|  x^0 |<----
        '-----'    +------+    +------+    '-----'    +------+
           |                                  ^
           |                                  |
           '----------------------------------'
                       feedback (fb)
 
    fb  = message_bit XOR x2   (leftmost (+))
    x2' = x1                   (plain wire -- G's x^2 coefficient is 0)
    x1' = x0 XOR fb            (middle (+), tap for G's x^1 coefficient)
    x0' = fb                   (tap for G's x^0 coefficient -- the same
                                 fb signal, just with no prior content to
                                 XOR against since nothing feeds x0 from
                                 further right; this is why the dangling
                                 arrow into x0 at the diagram's right edge
                                 has no (+) drawn on it)
  */
  int fb = message_bit ^ state.x2;
  crc_circuit_state next;
  next.x2 = state.x1;
  next.x1 = state.x0 ^ fb;
  next.x0 = fb;
  return next;
}

/* Processes all 8 bits of a byte, MSB first, in one call. */
crc_circuit_state crc_circuit_process_byte(crc_circuit_state state, uint8_t byte) {
  for (int i = 7; i >= 0; i--) {
    int bit = (byte >> i) & 1;
    state = crc_circuit_step(state, bit);
  }
  return state;
}

/* Packs (x2,x1,x0) into the low 3 bits of a uint8_t: x2<<2 | x1<<1 | x0. */
uint8_t crc_circuit_remainder(crc_circuit_state state) {
  return (uint8_t)((state.x2 << 2) | (state.x1 << 1) | state.x0);
}

#if defined(__aarch64__)
/*
  Same circuit, hand-written in ARM64 asm: w2 holds the packed
  3-bit state (x2 in bit 2, x1 in bit 1, x0 in bit 0), one 8-bit
  dividend processed per call, MSB first.
*/
uint8_t crc_circuit_remainder_asm(uint8_t dividend) {
  uint32_t result;
  __asm__ volatile (
    "mov   w2, #0              \n" /* w2 = state, packed x2:x1:x0 in bits 2:1:0 */
    "mov   w3, #7              \n" /* w3 = bit index, 7 downto 0 */
    "1%=:                      \n"
    "lsr   w4, %w[dividend], w3\n" /* w4 = dividend >> i */
    "and   w4, w4, #1          \n" /* w4 = message_bit */
    "lsr   w5, w2, #2          \n" /* w5 = old x2 */
    "eor   w4, w4, w5          \n" /* w4 = fb = message_bit ^ x2 */
    "lsr   w6, w2, #1          \n"
    "and   w6, w6, #1          \n" /* w6 = old x1 -> becomes new x2 */
    "and   w7, w2, #1          \n" /* w7 = old x0 */
    "eor   w7, w7, w4          \n" /* w7 = new x1 = old_x0 ^ fb */
    "orr   w2, w4, w7, lsl #1  \n" /* w2 = new_x0 | (new_x1 << 1) */
    "orr   w2, w2, w6, lsl #2  \n" /* w2 |= (new_x2 << 2) */
    "subs  w3, w3, #1          \n"
    "b.ge  1%=b                \n"
    "mov   %w[result], w2      \n"
    : [result] "=r" (result)
    : [dividend] "r" ((uint32_t)dividend)
    : "w2", "w3", "w4", "w5", "w6", "w7", "cc"
  );
  return (uint8_t)result;
}
#endif

void crc_circuit_print_trace(uint8_t dividend) {
  crc_circuit_state s = crc_circuit_init();
  printf("dividend 0x%02x, G = x^3+x+1\n", dividend);
  for (int i = 7; i >= 0; i--) {
    int bit = (dividend >> i) & 1;
    int fb = bit ^ s.x2;
    s = crc_circuit_step(s, bit);
    printf("  bit=%d fb=%d -> (x2,x1,x0)=(%d,%d,%d)\n", bit, fb, s.x2, s.x1, s.x0);
  }
  printf("remainder = (message * x^3) mod G = 0x%02x\n", crc_circuit_remainder(s));
}

#define G 0b1011u /* x^3+x+1 */

/*
  Known values for one step, read directly off a verified trace of
  the dividend 0xE6 (x^7+x^6+x^5+x^2+x, the book's own worked
  long-division example from earlier in this chapter).
*/
Test(crc_circuit, single_step_known_values) {
  crc_circuit_state s = crc_circuit_init();
  s = crc_circuit_step(s, 1);
  cr_assert_eq(s.x2, 0);
  cr_assert_eq(s.x1, 1);
  cr_assert_eq(s.x0, 1);

  s = crc_circuit_step(s, 1);
  cr_assert_eq(s.x2, 1);
  cr_assert_eq(s.x1, 0);
  cr_assert_eq(s.x0, 1);
}

Test(crc_circuit, book_dividend_example) {
  crc_circuit_state s = crc_circuit_process_byte(crc_circuit_init(), 0b11100110u);
  cr_assert_eq(crc_circuit_remainder(s), 0b100u, "got 0x%x", crc_circuit_remainder(s));
}

/*
  The defining feature of a CRC circuit: it computes
  (message * x^3) mod G, not plain (message mod G). Assert both
  explicitly for the same input so the distinction is tested, not
  just asserted in a comment -- they are genuinely different numbers
  (0b100 vs 0b101) for this exact dividend.
*/
Test(crc_circuit, differs_from_plain_division) {
  crc_circuit_state s = crc_circuit_process_byte(crc_circuit_init(), 0b11100110u);
  gf2_divmod_result plain = gf2_divide(0b11100110u, G);

  cr_assert_eq(crc_circuit_remainder(s), 0b100u);
  cr_assert_eq(plain.remainder, 0b101u);
  cr_assert_neq(
    crc_circuit_remainder(s), (uint8_t)plain.remainder,
    "CRC circuit and plain division should NOT agree here"
  );
}

/*
  Cross-check against gf2_divide, independently computing
  (dividend << 3) mod G, over every possible 8-bit dividend --
  the real claim this circuit makes, checked exhaustively rather
  than on a couple of hand-picked bytes.
*/
Test(crc_circuit, matches_gf2_divide_exhaustive) {
  for (uint32_t dividend = 0; dividend <= 0xFF; dividend++) {
    crc_circuit_state s = crc_circuit_process_byte(crc_circuit_init(), (uint8_t)dividend);
    gf2_divmod_result shifted = gf2_divide(dividend << 3, G);
    cr_assert_eq(
      crc_circuit_remainder(s), (uint8_t)shifted.remainder,
      "dividend=0x%02x circuit=0x%x gf2_divide=0x%x",
      dividend, crc_circuit_remainder(s), shifted.remainder
    );
  }
}

#if defined(__aarch64__)
Test(crc_circuit, asm_matches_c_exhaustive) {
  for (uint32_t dividend = 0; dividend <= 0xFF; dividend++) {
    crc_circuit_state s = crc_circuit_process_byte(crc_circuit_init(), (uint8_t)dividend);
    uint8_t c_result = crc_circuit_remainder(s);
    uint8_t asm_result = crc_circuit_remainder_asm((uint8_t)dividend);
    cr_assert_eq(
      c_result, asm_result,
      "dividend=0x%02x c=0x%x asm=0x%x", dividend, c_result, asm_result
    );
  }
}
#endif

/* No assertions used to watch each flip-flop update per bit */
Test(crc_circuit, trace) {
  printf("\n\n--start CRC circuit trace--\n");
  crc_circuit_print_trace(0b11100110u);
  printf("--end CRC circuit trace--\n\n");
/*
dividend 0xe6, G = x^3+x+1
  bit=1 fb=1 -> (x2,x1,x0)=(0,1,1)
  bit=1 fb=1 -> (x2,x1,x0)=(1,0,1)
  bit=1 fb=0 -> (x2,x1,x0)=(0,1,0)
  bit=0 fb=0 -> (x2,x1,x0)=(1,0,0)
  bit=0 fb=1 -> (x2,x1,x0)=(0,1,1)
  bit=1 fb=1 -> (x2,x1,x0)=(1,0,1)
  bit=1 fb=0 -> (x2,x1,x0)=(0,1,0)
  bit=0 fb=0 -> (x2,x1,x0)=(1,0,0)
remainder = (message * x^3) mod G = 0x04
*/
  cr_assert(1, "trace-only test");
}