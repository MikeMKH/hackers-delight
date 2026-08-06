#include <criterion/criterion.h>
#include <stdint.h>

#if defined(__aarch64__)

/*
  Signed division by 2^k using the four-instruction sequence from
  Hacker's Delight section 10-1. Implements q = n / 2^k (truncating).

  shrsi t, n, k-1   -- arithmetic shift right by k-1: all 1s if n<0, else 0
  shri  t, t, 32-k  -- logical shift right by 32-k: gives 2^k-1 if n<0, else 0
  add   t, n, t     -- add the bias to n
  shrsi q, t, k     -- arithmetic shift right by k: final quotient
 */
static int32_t divpow2_asm(int32_t n, int k) {
  int32_t result;
  __asm__ volatile (
    "asr  w2, %w[n], #31       \n"  /* t = n >>s 31: all 1s if n<0   */
    "lsr  w2, w2, %w[s32mk]    \n"  /* t = t >>u (32-k): gives 2^k-1 */
    "add  w2, %w[n], w2        \n"  /* t = n + bias                  */
    "asr  %w[q], w2, %w[k]     \n"  /* q = t >>s k                   */
    : [q]    "=r" (result)
    : [n]    "r"  (n),
      [k]    "r"  (k),
      [s32mk]"r"  (32 - k)
    : "w2"
  );
  return result;
}

/* C reference implementation of the same logic */
static int32_t divpow2_c(int32_t n, int k) {
  /* bias = 2^k - 1 if n < 0, else 0 */
  int32_t bias = (n >> 31) & ((1u << k) - 1);
  return (n + bias) >> k;
}

static void check_divpow2(int32_t n, int k) {
  int32_t expected = divpow2_c(n, k);
  int32_t got_asm  = divpow2_asm(n, k);
  int32_t ref      = (k < 31) ? (n / (int32_t)(1u << k)) : (n == (int32_t)0x80000000 ? -1 : 0);
  cr_assert_eq(
    got_asm, expected,
    "asm mismatch: divpow2(%d, %d) got %d want %d", n, k, got_asm, expected
  );
  cr_assert_eq(
    got_asm, ref,
    "wrong result: divpow2(%d, %d) got %d want %d", n, k, got_asm, ref);
}

Test(divpow2, k_equals_1) {
  /* division by 2, simplifies to 3 instructions */
  check_divpow2( 7, 1);   /*  7/2 =  3 */
  check_divpow2(-7, 1);   /* -7/2 = -3 (truncating toward zero) */
  check_divpow2( 6, 1);   /*  6/2 =  3 */
  check_divpow2(-6, 1);   /* -6/2 = -3 */
  check_divpow2( 1, 1);
  check_divpow2(-1, 1);
  check_divpow2( 0, 1);
}

Test(divpow2, k_equals_2) {
  check_divpow2(  7, 2);  /*  7/4 =  1 */
  check_divpow2( -7, 2);  /* -7/4 = -1 */
  check_divpow2( 12, 2);  /* 12/4 =  3 */
  check_divpow2(-12, 2);  /*-12/4 = -3 */
}

Test(divpow2, k_equals_4) {
  check_divpow2( 100, 4);  /* 100/16 =  6 */
  check_divpow2(-100, 4);  /*-100/16 = -6 */
  check_divpow2(  16, 4);  /*  16/16 =  1 */
  check_divpow2( -16, 4);  /* -16/16 = -1 */
  check_divpow2(  15, 4);  /*  15/16 =  0 */
  check_divpow2( -15, 4);  /* -15/16 =  0 */
}

Test(divpow2, negative_bias_applied) {
  /*
    key property: negative numbers get bias added before shift,
    so truncation goes toward zero not toward negative infinity
  */
  check_divpow2(-1, 2);   /* -1/4 = 0, not -1 */
  check_divpow2(-3, 2);   /* -3/4 = 0, not -1 */
  check_divpow2(-4, 2);   /* -4/4 = -1 */
  check_divpow2(-5, 2);   /* -5/4 = -1, not -2 */
}

Test(divpow2, k_equals_31) {
  /* edge case: k=31, 2^31 not representable, special cases in text */
  int32_t n_min = (int32_t)0x80000000;       /* -2^31 */
  cr_assert_eq(divpow2_asm(n_min, 31), -1);  /* -2^31 / 2^31 = -1 */
  cr_assert_eq(divpow2_asm( 0, 31),     0);
  cr_assert_eq(divpow2_asm( 1, 31),     0);
  cr_assert_eq(divpow2_asm(-1, 31),     0);
}

Test(divpow2, agrees_with_c_reference) {
  int32_t ns[] = {
    0, 1, -1, 7, -7, 100, -100,
    0x7FFFFFFF, (int32_t)0x80000000
  };
  int ks[] = { 1, 2, 3, 4, 8, 16, 31 };
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++) {
    for (size_t j = 0; j < sizeof ks / sizeof ks[0]; j++) {
      check_divpow2(ns[i], ks[j]);
    }
  }
}

#else
Test(divpow2, skipped) { cr_skip("ARM AArch64 only"); }
#endif