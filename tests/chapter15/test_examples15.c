#include <criterion/criterion.h>
#include <stdint.h>
#include <stdio.h>

/* Deterministic pseudo-random words so failures are reproducible. */
static uint32_t xorshift32(uint32_t *state) {
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

/*
  Six parity check bits for the 32 "information" bits in u, returned in
  the low six bits (p[5:0]). An overall parity bit is meant to be
  prepended by some other process; this function does not compute it.
  See checkbits.c for how the check bits are defined and computed.
*/
uint32_t checkbits(uint32_t u) {
  /*
    Each check bit is the parity (xor) of a fixed set of bits of u.
    Written as masks, with u[0] set aside for the moment:

      p[0]  u & 0xAAAAAAAA   indices with bit 0 set  1, 3, 5, ..., 31
      p[1]  u & 0xCCCCCCCC   indices with bit 1 set  2-3, 6-7, ..., 30-31
      p[2]  u & 0xF0F0F0F0   indices with bit 2 set  4-7, 12-15, ..., 28-31
      p[3]  u & 0xFF00FF00   indices with bit 3 set  8-15, 24-31
      p[4]  u & 0xFFFF0000   indices with bit 4 set  16-31
      p[5]  u & 0xFFFFFFFE   every index except 0

    u[0] has no bits set in its index, so it is covered by hand: it
    belongs to p[0] through p[4] but not to p[5].

    The shifts compute those six parities without any loop. Each
    "x ^ (x >> n)" folds the word onto itself, like one level of the
    exclusive-or tree in Figure 14-1, except the folds stride across the
    word instead of halving it. After the folds, bit j of the word holds
    the xor of a whole pattern of u starting at index j, and the answer
    is read from the one bit whose pattern is exactly the mask above:
    bit 1 for p[0], 2 for p[1], 4 for p[2], 8 for p[3], 16 for p[4], and
    0 for p[5]. t1, t2 and t3 are partial results shared between chains.
  */
  uint32_t p0, p1, p2, p3, p4, p5, p;
  uint32_t t1, t2, t3;

  /* First calculate p[5:0] ignoring u[0]. */
  p0 = u ^ (u >> 2);
  p0 = p0 ^ (p0 >> 4);
  p0 = p0 ^ (p0 >> 8);
  p0 = p0 ^ (p0 >> 16);        /* p0 is in posn 1. */

  t1 = u ^ (u >> 1);
  p1 = t1 ^ (t1 >> 4);
  p1 = p1 ^ (p1 >> 8);
  p1 = p1 ^ (p1 >> 16);        /* p1 is in posn 2. */

  t2 = t1 ^ (t1 >> 2);
  p2 = t2 ^ (t2 >> 8);
  p2 = p2 ^ (p2 >> 16);        /* p2 is in posn 4. */

  t3 = t2 ^ (t2 >> 4);
  p3 = t3 ^ (t3 >> 16);        /* p3 is in posn 8. */

  p4 = t3 ^ (t3 >> 8);         /* p4 is in posn 16. */

  p5 = p4 ^ (p4 >> 16);        /* p5 is in posn 0. */

  p = ((p0 >> 1) & 1u) | ((p1 >> 1) & 2u) | ((p2 >> 2) & 4u) |
      ((p3 >> 5) & 8u) | ((p4 >> 12) & 16u) | ((p5 & 1u) << 5);

  /*
    Now account for u[0]. -(u & 1u) is all ones when u[0] is 1 and all
    zeros when it is 0 (well-defined unsigned wraparound, the same mask
    trick as the CRC-32 loop). Flipping all six bits adds u[0] to p[0]
    through p[4]. p5's chain happened to include u[0] already, so the
    same flip cancels it back out of p[5].
  */
  p = p ^ (-(u & 1u) & 0x3Fu);
  return p;
}

static void print_binary(const char *label, uint32_t x) {
  printf("  %-10s ", label);
  for (int i = 31; i >= 0; i--) {
    putchar(((x >> i) & 1u) ? '1' : '0');
    if (i % 4 == 0 && i != 0) putchar('_');
  }
  printf("  0x%08x\n", x);
}

/*
  Same computation as checkbits(), printing every intermediate word in
  binary so the shift-and-xor folds can be watched. Returns the result
  so a test can confirm the trace never drifts from checkbits().
*/
uint32_t checkbits_print_trace(uint32_t u) {
  uint32_t p0, p1, p2, p3, p4, p5, p;
  uint32_t t1, t2, t3;

  printf("checkbits trace, u = 0x%08x\n", u);
  printf("             bit 31                          bit 0\n");
  print_binary("u", u);

  printf("p[0] chain: folds with strides 2, 4, 8, 16; answer in bit 1\n");
  p0 = u ^ (u >> 2);   print_binary("u^(u>>2)", p0);
  p0 = p0 ^ (p0 >> 4); print_binary("^(>>4)", p0);
  p0 = p0 ^ (p0 >> 8); print_binary("^(>>8)", p0);
  p0 = p0 ^ (p0 >> 16); print_binary("^(>>16)", p0);

  printf("shared partial results\n");
  t1 = u ^ (u >> 1);   print_binary("t1", t1);
  t2 = t1 ^ (t1 >> 2); print_binary("t2", t2);
  t3 = t2 ^ (t2 >> 4); print_binary("t3", t3);

  printf("remaining chains\n");
  p1 = t1 ^ (t1 >> 4);
  p1 = p1 ^ (p1 >> 8);
  p1 = p1 ^ (p1 >> 16); print_binary("p1 (bit 2)", p1);
  p2 = t2 ^ (t2 >> 8);
  p2 = p2 ^ (p2 >> 16); print_binary("p2 (bit 4)", p2);
  p3 = t3 ^ (t3 >> 16); print_binary("p3 (bit 8)", p3);
  p4 = t3 ^ (t3 >> 8);  print_binary("p4 (bit 16)", p4);
  p5 = p4 ^ (p4 >> 16); print_binary("p5 (bit 0)", p5);

  p = ((p0 >> 1) & 1u) | ((p1 >> 1) & 2u) | ((p2 >> 2) & 4u) |
      ((p3 >> 5) & 8u) | ((p4 >> 12) & 16u) | ((p5 & 1u) << 5);
  printf("gathered p[5:0] before the u[0] fix-up: 0x%02x\n", p);

  p = p ^ (-(u & 1u) & 0x3Fu);
  printf("after the u[0] fix-up (u[0] = %u):       0x%02x\n", u & 1u, p);
  return p;
}

/*
  Every check bit is a parity over a fixed set of bits of u, so
  checkbits() is linear over GF(2): each bit u[i] contributes a fixed
  6-bit "signature" to the answer, and the answer for any u is the xor
  of the signatures of the bits that are set. These tests build up from
  that picture: the signatures, why they make single errors findable,
  an independent oracle, linearity, and finally a trace.

  Signature of u[i], as p[5:0]:
    u[0]   covered by p[0]..p[4], not p[5]           0b011111 = 0x1F
    u[i]   (i = 1..31) covered by p[5], plus p[k]
           for every bit k set in i                  0b1 followed by i's 5 bits
                                                      = 0x20 | i
*/
static const uint32_t SIGNATURE[32] = {
  /* i =  0 */     0x1F,
  /* i =  1..7 */  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  /* i =  8..15 */ 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  /* i = 16..23 */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  /* i = 24..31 */ 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

/*
  Independent oracle: the six masks from the comment in checkbits.c and
  the compiler's own parity builtin, with no shift-and-xor network.
*/
static uint32_t checkbits_oracle(uint32_t u) {
  static const uint32_t MASK[5] = {
    0xAAAAAAAAu, 0xCCCCCCCCu, 0xF0F0F0F0u, 0xFF00FF00u, 0xFFFF0000u
  };
  uint32_t p = 0;
  for (int k = 0; k < 5; k++) {
    uint32_t bit = (uint32_t)__builtin_parity(u & MASK[k]) ^ (u & 1u);
    p |= bit << k; /* p[0..4] also cover u[0] */
  }
  p |= (uint32_t)__builtin_parity(u & 0xFFFFFFFEu) << 5; /* p[5] skips u[0] */
  return p;
}

/*
  Hand-checkable values. Each is the xor of the signatures of the set
  bits, e.g. u = 3 sets u[0] and u[1]: 0x1F ^ 0x21 = 0x3E.
*/
Test(checkbits, known_values) {
  cr_assert_eq(checkbits(0x00000000u), 0x00u);
  cr_assert_eq(checkbits(0x00000001u), 0x1Fu, "u[0] alone");
  cr_assert_eq(checkbits(0x00000002u), 0x21u, "u[1] alone");
  cr_assert_eq(checkbits(0x00000003u), 0x3Eu, "u[0] and u[1]");
  cr_assert_eq(checkbits(0x00000021u), 0x3Au, "u[0] and u[5]: 0x1F ^ 0x25");
  cr_assert_eq(checkbits(0x80000000u), 0x3Fu, "u[31] alone");
  cr_assert_eq(checkbits(0xFFFFFFFFu), 0x3Fu, "all ones");
}

/* Feed in one bit at a time and read each bit's signature off the output. */
Test(checkbits, each_information_bit_has_its_own_signature) {
  for (int i = 0; i < 32; i++) {
    uint32_t got = checkbits(1u << i);
    cr_assert_eq(
      got, SIGNATURE[i],
      "u[%d]: got 0x%02x want 0x%02x", i, got, SIGNATURE[i]
    );
  }
}

/* Signature (i = 0..31) or check bit p[k] itself (i = 32..37, signature 1<<k). */
static uint32_t column(int pos) {
  return pos < 32 ? SIGNATURE[pos] : (1u << (pos - 32));
}

/*
  Why the signatures are what they are: for a code to locate a single
  flipped bit, every bit of the 38-bit word (32 information + 6 check)
  must leave a different, nonzero fingerprint when it flips.
*/
Test(checkbits, all_38_single_bit_fingerprints_are_distinct_and_nonzero) {
  for (int a = 0; a < 38; a++) {
    cr_assert_neq(
      column(a), 0u,
      "position %d has a zero fingerprint", a
    );
    for (int b = a + 1; b < 38; b++) {
      cr_assert_neq(
        column(a), column(b),
        "positions %d and %d collide", a, b
      );
    }
  }
}

static int locate(uint32_t syndrome) {
  for (int pos = 0; pos < 38; pos++) {
    if (column(pos) == syndrome) return pos;
  }
  return -1;
}

/*
  The receiver's side: recompute checkbits() from the received information
  bits and xor with the received check bits. That syndrome is zero for a
  clean word and equals the flipped bit's fingerprint otherwise.
*/
Test(checkbits, syndrome_locates_any_single_bit_error) {
  uint32_t seed = 0x2545F491u;
  for (int n = 0; n < 200; n++) {
    uint32_t u = xorshift32(&seed);
    uint32_t p = checkbits(u);
    cr_assert_eq(
      checkbits(u) ^ p, 0u,
      "clean word u=0x%08x had a nonzero syndrome", u
    );

    for (int pos = 0; pos < 38; pos++) {
      uint32_t bad_u = u;
      uint32_t bad_p = p;
      if (pos < 32) bad_u ^= 1u << pos;
      else          bad_p ^= 1u << (pos - 32);

      uint32_t syndrome = checkbits(bad_u) ^ bad_p;
      cr_assert_eq(
        syndrome, column(pos),
        "u=0x%08x flipped position %d: syndrome 0x%02x want 0x%02x",
        u, pos, syndrome, column(pos)
      );
      cr_assert_eq(
        locate(syndrome), pos,
        "syndrome 0x%02x located the wrong bit", syndrome
      );
    }
  }
}

/*
  Cross-check against the mask-and-parity oracle: every 32-bit word whose
  upper half is zero, every word whose lower half is zero, all one- and
  two-bit words, and 200000 pseudo-random words.
*/
Test(checkbits, matches_mask_and_parity_oracle) {
  for (uint32_t v = 0; v <= 0xFFFFu; v++) {
    cr_assert_eq(
      checkbits(v), checkbits_oracle(v),
      "low half u=0x%08x", v
    );
    cr_assert_eq(
      checkbits(v << 16), checkbits_oracle(v << 16),
      "high half u=0x%08x", v << 16
    );
  }
  for (int i = 0; i < 32; i++) {
    for (int j = i; j < 32; j++) {
      uint32_t u = (1u << i) | (1u << j);
      cr_assert_eq(
        checkbits(u), checkbits_oracle(u),
        "bits %d,%d u=0x%08x", i, j, u
      );
    }
  }
  uint32_t seed = 0xC0FFEE11u;
  for (int n = 0; n < 200000; n++) {
    uint32_t u = xorshift32(&seed);
    cr_assert_eq(
      checkbits(u), checkbits_oracle(u),
      "random u=0x%08x", u
    );
  }
}

/*
  Linearity over GF(2): checkbits(a ^ b) == checkbits(a) ^ checkbits(b).
  This is the reason the 32 single-bit tests above pin the function down
  for every input, and the reason the whole thing can be built from xors
  and shifts in the first place.
*/
Test(checkbits, is_linear_over_gf2) {
  uint32_t seed = 0xDEADBEEFu;
  for (int n = 0; n < 100000; n++) {
    uint32_t a = xorshift32(&seed);
    uint32_t b = xorshift32(&seed);
    cr_assert_eq(
      checkbits(a ^ b), checkbits(a) ^ checkbits(b),
      "a=0x%08x b=0x%08x", a, b
    );
  }
}

/*
  No new assertions used to watch the folds.
*/
Test(checkbits, trace) {
  printf("\n\n--start checkbits trace--\n");
  cr_assert_eq(
    checkbits_print_trace(0x00000021u), checkbits(0x00000021u)
  );
  printf("--end checkbits trace--\n\n");
/*
checkbits trace, u = 0x00000021
             bit 31                          bit 0
  u          0000_0000_0000_0000_0000_0000_0010_0001  0x00000021
p[0] chain: folds with strides 2, 4, 8, 16; answer in bit 1
  u^(u>>2)   0000_0000_0000_0000_0000_0000_0010_1001  0x00000029
  ^(>>4)     0000_0000_0000_0000_0000_0000_0010_1011  0x0000002b
  ^(>>8)     0000_0000_0000_0000_0000_0000_0010_1011  0x0000002b
  ^(>>16)    0000_0000_0000_0000_0000_0000_0010_1011  0x0000002b
shared partial results
  t1         0000_0000_0000_0000_0000_0000_0011_0001  0x00000031
  t2         0000_0000_0000_0000_0000_0000_0011_1101  0x0000003d
  t3         0000_0000_0000_0000_0000_0000_0011_1110  0x0000003e
remaining chains
  p1 (bit 2) 0000_0000_0000_0000_0000_0000_0011_0010  0x00000032
  p2 (bit 4) 0000_0000_0000_0000_0000_0000_0011_1101  0x0000003d
  p3 (bit 8) 0000_0000_0000_0000_0000_0000_0011_1110  0x0000003e
  p4 (bit 16) 0000_0000_0000_0000_0000_0000_0011_1110  0x0000003e
  p5 (bit 0) 0000_0000_0000_0000_0000_0000_0011_1110  0x0000003e
gathered p[5:0] before the u[0] fix-up: 0x05
after the u[0] fix-up (u[0] = 1):       0x3a
*/
 cr_assert(1, "trace-only test");
}

/*
  Parity of a 32-bit word: 1 if an odd number of bits are set, 0 if even.
  Three more levels of the exclusive-or tree from Figure 14-1.
*/
uint32_t parity32(uint32_t x) {
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return x & 1u;
}

/*
  The receiver's side of the 39-bit code (32 information bits, 6 check
  bits, 1 overall parity bit). Decides how many errors occurred and, if
  exactly one of them was in the information word, fixes it in place.
 
  pr  the received check word. Bits 5:0 are the check bits as sent, bit 6
      is the overall parity bit, chosen by the sender so that all 39 bits
      together have even parity. Bits 31:7 must be zero.
  ur  the received information word. Corrected in place only when a
      single error was found in it.
 
  Returns the number of errors found: 0, 1 (corrected, or in the check
  bits and harmless to ur), or 2 (detected, NOT corrected, ur untouched).
 
  Domain: this is only right when at most 2 of the 39 received bits were
  flipped, the same presumption the book makes. With 3 or more flips the
  answer can be wrong (a double error reported as one, or 4 errors as 0),
  and a syndrome that is no single bit's fingerprint can make the shift
  count out of range, which is undefined behavior. This function does not
  guard against that; callers that cannot rule it out should.
*/
int correct(uint32_t pr, uint32_t *ur) {
  /*
    Two separate pieces of evidence, then a decision.

    po   parity of all 39 received bits. The sender made it even, so po is
         0 after an even number of flips and 1 after an odd number.
    syn  the syndrome: the check bits recomputed from the received
         information, xor the check bits that were received. It is 0 for a
         clean word and otherwise the fingerprint of the flipped bit (for
         two flips, the xor of two fingerprints). The overall parity bit
         is left out of it (pr & 0x3F).

    po = 0, syn = 0   nothing wrong                              -> 0
    po = 0, syn != 0  even number of flips, but not zero: two    -> 2
    po = 1            odd number of flips, taken to be one:      -> 1
  */
  uint32_t po, p, syn, b;

  po = parity32(pr ^ *ur);
  p = checkbits(*ur);
  syn = p ^ (pr & 0x3Fu);

  if (po == 0) {
    if (syn == 0) return 0;
    else return 2;
  }

  /*
    One error. (syn - 1) & syn clears the lowest set bit, so it is zero
    exactly when syn is 0 or a power of two. A check bit p[k] has
    fingerprint 1 << k, and the overall parity bit leaves syn at 0, so
    either way the damage is outside ur and there is nothing to fix.
    No information bit has a power of two as its fingerprint (they are 31
    and 33..63), which is what makes this test safe.
  */
  if (((syn - 1u) & syn) == 0) return 1;

  /*
    Otherwise syn is an information bit's fingerprint: 31 for u[0], and
    32 + i for u[i] with i = 1..31. This maps it back to i. For syn = 31,
    syn >> 5 is 0 and b = 0. For syn = 33..63, syn >> 5 is 1 and
    b = syn - 32.
  */
  b = syn - 31u - (syn >> 5);
  *ur = *ur ^ (1u << b);   /* 1u, not 1: b = 31 would overflow a signed int */
  return 1;
}

/*
  Returns what correct() would return and updates *ur the same way.
*/
int correct_print_trace(uint32_t pr, uint32_t *ur) {
  uint32_t po = parity32(pr ^ *ur);
  uint32_t p = checkbits(*ur);
  uint32_t syn = p ^ (pr & 0x3Fu);

  printf("  received: pr = 0x%02x  ur = 0x%08x\n", pr, *ur);
  printf("  po  (parity of all 39 bits)        = %u\n", po);
  printf("  p   (checkbits of received ur)     = 0x%02x\n", p);
  printf("  syn (p ^ received check bits)      = 0x%02x\n", syn);

  if (po == 0) {
    if (syn == 0) {
      printf("  -> po = 0, syn = 0: no errors\n");
      return 0;
    }
    printf("  -> po = 0, syn != 0: two errors, detected but not correctable\n");
    return 2;
  }

  if (((syn - 1u) & syn) == 0) {
    if (syn == 0) printf("  -> po = 1, syn = 0: the overall parity bit itself was flipped\n");
    else          printf("  -> po = 1, syn is a power of two: check bit p[%d] was flipped\n",
                         __builtin_ctz(syn));
    printf("     ur is fine, nothing to correct\n");
    return 1;
  }

  uint32_t b = syn - 31u - (syn >> 5);
  printf("  -> po = 1, syn is an information bit's fingerprint\n");
  printf("     b = syn - 31 - (syn >> 5) = %u, so flip u[%u]\n", b, b);
  *ur = *ur ^ (1u << b);
  printf("     corrected ur = 0x%08x\n", *ur);
  return 1;
}

/*
  The receiver sees 39 bits: the 32 information bits u[0..31], the six
  check bits p[0..5], and one overall parity bit. These tests number
  them as positions 0..38:

    0..31   u[0..31]  (the information word, ur)
    32..37  p[0..5]   (bits 0..5 of the check word, pr)
    38      the overall parity bit (bit 6 of pr)

  correct() has to sort every possible damage pattern into three piles:
  nothing wrong (0), one flip (1), two flips (2).
*/


/*
  Build pr from u. Bits 5:0 are checkbits(u); bit 6 is chosen so that all 39
  bits together have even parity. Uses the compiler's parity builtin so it
  does not depend on parity32() under test.
*/
static uint32_t make_pr(uint32_t u) {
  uint32_t p = checkbits(u);
  uint32_t overall = (uint32_t)__builtin_parity(u) ^ (uint32_t)__builtin_parity(p);
  return (overall << 6) | p;
}

static void flip(uint32_t *pr, uint32_t *ur, int pos) {
  if (pos < 32) *ur ^= 1u << pos;
  else          *pr ^= 1u << (pos - 32);   /* 32..37 -> pr bits 0..5, 38 -> bit 6 */
}

/* The syndrome as correct() sees it. */
static uint32_t syndrome_of(uint32_t pr, uint32_t ur) {
  return checkbits(ur) ^ (pr & 0x3Fu);
}

Test(receiver, parity32_matches_builtin) {
  cr_assert_eq(parity32(0x00000000u), 0u);
  cr_assert_eq(parity32(0x00000001u), 1u);
  cr_assert_eq(parity32(0x00000003u), 0u);
  cr_assert_eq(parity32(0x80000000u), 1u);
  cr_assert_eq(parity32(0xFFFFFFFFu), 0u, "32 ones is an even count");

  for (uint32_t v = 0; v <= 0xFFFFu; v++) {
    cr_assert_eq(parity32(v), (uint32_t)__builtin_parity(v), "v=0x%08x", v);
    cr_assert_eq(parity32(v << 16), (uint32_t)__builtin_parity(v << 16), "v=0x%08x", v << 16);
  }
  uint32_t seed = 0x1234ABCDu;
  for (int n = 0; n < 100000; n++) {
    uint32_t x = xorshift32(&seed);
    cr_assert_eq(parity32(x), (uint32_t)__builtin_parity(x), "x=0x%08x", x);
  }
}

/* po = 0 and syn = 0: nothing was flipped, so nothing may change. */
Test(receiver, clean_word_reports_zero_errors_and_changes_nothing) {
  uint32_t seed = 0x2545F491u;
  for (int n = 0; n < 2000; n++) {
    uint32_t u = (n == 0) ? 0u : (n == 1) ? 0xFFFFFFFFu : xorshift32(&seed);
    uint32_t ur = u;
    int r = correct(make_pr(u), &ur);
    cr_assert_eq(r, 0, "u=0x%08x", u);
    cr_assert_eq(ur, u, "a clean word was modified: u=0x%08x ur=0x%08x", u, ur);
  }
}

/*
  One flipped information bit: po = 1 (odd number of flips) and syn is that
  bit's fingerprint, which correct() turns back into a bit number and flips
  back. All 32 positions, on 100 different words.
*/
Test(receiver, single_error_in_any_information_bit_is_corrected) {
  uint32_t seed = 0xC0FFEE11u;
  for (int n = 0; n < 100; n++) {
    uint32_t u = xorshift32(&seed);
    for (int i = 0; i < 32; i++) {
      uint32_t ur = u ^ (1u << i);
      int r = correct(make_pr(u), &ur);
      cr_assert_eq(r, 1, "u=0x%08x bit %d", u, i);
      cr_assert_eq(ur, u, "u=0x%08x bit %d not repaired: got 0x%08x", u, i, ur);
    }
  }
}

/*
  u[0] is the odd one out: its fingerprint is 31 (0b011111), where every
  other u[i] has 32 + i. The mapping has to special-case it, and does it
  with arithmetic: for syn = 31, syn >> 5 is 0 so b = 31 - 31 - 0 = 0.
*/
Test(receiver, bit_0_is_the_special_case_in_the_mapping) {
  uint32_t u = 0x12345678u;
  uint32_t pr = make_pr(u);
  uint32_t ur = u ^ 1u;
  cr_assert_eq(syndrome_of(pr, ur), 0x1Fu);
  cr_assert_eq(correct(pr, &ur), 1);
  cr_assert_eq(ur, u);
}

/*
  u[31] has the largest fingerprint, 63, so b = 63 - 31 - 1 = 31 and the
  repair is "ur ^ (1u << 31)".
*/
Test(receiver, bit_31_correction_needs_an_unsigned_shift) {
  uint32_t u = 0x12345678u;
  uint32_t pr = make_pr(u);
  uint32_t ur = u ^ 0x80000000u;
  cr_assert_eq(syndrome_of(pr, ur), 0x3Fu);
  cr_assert_eq(correct(pr, &ur), 1);
  cr_assert_eq(ur, u);
}

/*
  Checked against the real fingerprints coming out of
  checkbits(): flipping u[i] in the all-zero word leaves syn = checkbits(1 << i).
*/
Test(receiver, syndrome_to_bit_mapping_inverts_the_fingerprints) {
  for (uint32_t i = 0; i < 32; i++) {
    uint32_t syn = checkbits(1u << i);
    uint32_t one_line = syn - 31u - (syn >> 5);
    uint32_t two_line = (syn == 0x1Fu) ? 0u : (syn & 0x1Fu);
    cr_assert_eq(
      one_line, i,
      "u[%u]: syn=0x%02x one-line gave %u", i, syn, one_line
    );
    cr_assert_eq(
      two_line, i,
      "u[%u]: syn=0x%02x two-line gave %u", i, syn, two_line
    );
  }
}

/*
  A flip in a check bit or in the overall parity bit also makes po = 1, but
  the syndrome is then 0 (overall bit) or a power of two (p[k] has
  fingerprint 1 << k). (syn - 1) & syn is zero for exactly those, so
  correct() answers 1 and leaves ur alone.
*/
Test(receiver, error_in_check_bit_or_overall_parity_is_reported_and_ur_is_untouched) {
  uint32_t seed = 0xBADC0DE5u;
  for (int n = 0; n < 200; n++) {
    uint32_t u = xorshift32(&seed);
    for (int pos = 32; pos <= 38; pos++) {
      uint32_t pr = make_pr(u);
      uint32_t ur = u;
      flip(&pr, &ur, pos);

      uint32_t syn = syndrome_of(pr, ur);
      uint32_t want_syn = (pos == 38) ? 0u : (1u << (pos - 32));
      cr_assert_eq(
        syn, want_syn,
        "u=0x%08x pos %d: syn=0x%02x", u, pos, syn
      );
      cr_assert_eq(
        correct(pr, &ur), 1,
        "u=0x%08x pos %d", u, pos
      );
      cr_assert_eq(
        ur, u,
        "u=0x%08x pos %d: ur was modified", u, pos
      );
    }
  }
}

/*
  Two flips anywhere among the 39 bits: po goes back to 0 but the syndrome
  is not 0, because the two fingerprints are different (or one is the
  overall bit, whose fingerprint is 0, and the other is not). The answer is
  2, and ur must be left exactly as received: with two errors there is no
  telling which bits to repair, so correct() does not try. All 741 pairs.
*/
Test(receiver, every_double_error_is_detected_and_ur_is_left_alone) {
  uint32_t seed = 0xFEEDF00Du;
  for (int n = 0; n < 50; n++) {
    uint32_t u = xorshift32(&seed);
    for (int a = 0; a < 39; a++) {
      for (int b = a + 1; b < 39; b++) {
        uint32_t pr = make_pr(u);
        uint32_t ur = u;
        flip(&pr, &ur, a);
        flip(&pr, &ur, b);
        uint32_t received_ur = ur;

        cr_assert_eq(
          correct(pr, &ur), 2,
          "u=0x%08x flips %d,%d", u, a, b
        );
        cr_assert_eq(
          ur, received_ur,
          "u=0x%08x flips %d,%d: ur was modified", u, a, b
        );
      }
    }
  }
}

/*
  The limit of the code. With three flips po is 1, so correct() assumes one
  error and acts on the syndrome, which is now the xor of three fingerprints
  and can look like something else. Both examples below return 1, "one error",
  and leave the word wrong.
*/
Test(receiver, three_errors_are_outside_the_contract) {
  uint32_t u = 0x12345678u;

  /* u[1], u[2], u[4]: 0x21 ^ 0x22 ^ 0x24 = 0x27, which is u[7]'s fingerprint. */
  uint32_t pr = make_pr(u);
  uint32_t ur = u ^ (1u << 1) ^ (1u << 2) ^ (1u << 4);
  cr_assert_eq(correct(pr, &ur), 1);
  cr_assert_eq(
    ur, u ^ (1u << 1) ^ (1u << 2) ^ (1u << 4) ^ (1u << 7),
    "it 'repaired' u[7], leaving four wrong bits"
  );

  /* u[1], u[2], u[3]: 0x21 ^ 0x22 ^ 0x23 = 0x20, which looks like p[5] flipped. */
  pr = make_pr(u);
  ur = u ^ (1u << 1) ^ (1u << 2) ^ (1u << 3);
  uint32_t received_ur = ur;
  cr_assert_eq(correct(pr, &ur), 1);
  cr_assert_eq(
    ur, received_ur,
    "reported as a check-bit error, so ur was left with three wrong bits"
  );
}

static void trace_case(const char *title, uint32_t pr, uint32_t ur) {
  uint32_t for_trace = ur;
  uint32_t for_plain = ur;
  printf("%s\n", title);
  int traced = correct_print_trace(pr, &for_trace);
  int plain = correct(pr, &for_plain);
  cr_assert_eq(traced, plain, "%s", title);
  cr_assert_eq(for_trace, for_plain, "%s", title);
}

/*
  No new assertions used to watch each decision path.
*/
Test(receiver, trace) {
  printf("\n\n--start correct trace--\n");
  
  uint32_t u = 0x12345678u;
  uint32_t pr, ur;

  pr = make_pr(u); ur = u;
  trace_case("1. clean word", pr, ur);

  pr = make_pr(u); ur = u; flip(&pr, &ur, 6);
  trace_case("2. information bit u[6] flipped", pr, ur);

  pr = make_pr(u); ur = u; flip(&pr, &ur, 0);
  trace_case("3. information bit u[0] flipped (the special case)", pr, ur);

  pr = make_pr(u); ur = u; flip(&pr, &ur, 32 + 2);
  trace_case("4. check bit p[2] flipped", pr, ur);

  pr = make_pr(u); ur = u; flip(&pr, &ur, 38);
  trace_case("5. overall parity bit flipped", pr, ur);

  pr = make_pr(u); ur = u; flip(&pr, &ur, 6); flip(&pr, &ur, 20);
  trace_case("6. two flips, u[6] and u[20]", pr, ur);
  
  printf("--end correct trace--\n\n");
}