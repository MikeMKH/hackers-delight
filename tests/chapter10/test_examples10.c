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

#if defined(__aarch64__)

/*
  Signed remainder by 2^k using the instruction sequence from
  Hacker's Delight section 10-2. Implements r = n % 2^k (truncating).

  shrsi t, n, k-1   -- form the integer
  shri  t, t, 32-k  -- 2**k - 1 if n < 0, else 0
  add   t, n, t     -- add it to n
  andi  t, t, -2**k -- clear rightmost k bits
  sub   r, n, t     -- and subtract it from n
 */
static int32_t rempow2_asm(int32_t n, int k) {
  int32_t result;
  uint32_t mask = -(1u << k);  /* -2^k: clears low k bits */
  __asm__ volatile (
    "asr  w2, %w[n], #31       \n"  /* t = all 1s if n<0, else 0     */
    "lsr  w2, w2, %w[s32mk]    \n"  /* t = 2^k-1 if n<0, else 0      */
    "add  w2, %w[n], w2        \n"  /* t = n + bias                   */
    "and  w2, w2, %w[mask]     \n"  /* t = t & -2^k (clear low k bits)*/
    "sub  %w[q], %w[n], w2     \n"  /* r = n - t                      */
    : [q]    "=r" (result)
    : [n]    "r"  (n),
      [s32mk]"r"  (32 - k),
      [mask] "r"  (mask)
    : "w2"
  );
  return result;
}

/* C reference implementation of the same logic */
static int32_t rempow2_c(int32_t n, int k) {
  /* bias = 2^k - 1 if n < 0, else 0 */
  int32_t bias = (n >> 31) & ((1u << k) - 1);
  int32_t t = (n + bias) & -(1u << k);  /* round down to multiple of 2^k */
  return n - t;
}

static void check_rempow2(int32_t n, int k) {
  int32_t expected = rempow2_c(n, k);
  int32_t got_asm  = rempow2_asm(n, k);
  int32_t ref      = (k < 31) ? (n % (int32_t)(1u << k))
                              : (n == (int32_t)0x80000000 ? 0 : n % 2);
  cr_assert_eq(
    got_asm, expected,
    "asm vs c mismatch: rempow2(%d, %d) asm=%d c=%d", n, k, got_asm, expected
  );
  cr_assert_eq(
    got_asm, ref,
    "wrong result: rempow2(%d, %d) got %d want %d", n, k, got_asm, ref
  );
}

Test(rempow2, k_equals_1) {
  check_rempow2( 7, 1);   /*  7 % 2 =  1 */
  check_rempow2(-7, 1);   /* -7 % 2 = -1 */
  check_rempow2( 6, 1);   /*  6 % 2 =  0 */
  check_rempow2(-6, 1);   /* -6 % 2 =  0 */
  check_rempow2( 1, 1);
  check_rempow2(-1, 1);
  check_rempow2( 0, 1);
}

Test(rempow2, k_equals_2) {
  check_rempow2(  7, 2);  /*  7 % 4 =  3 */
  check_rempow2( -7, 2);  /* -7 % 4 = -3 */
  check_rempow2( 12, 2);  /* 12 % 4 =  0 */
  check_rempow2(-12, 2);  /*-12 % 4 =  0 */
  check_rempow2( -1, 2);  /* -1 % 4 = -1 */
  check_rempow2( -3, 2);  /* -3 % 4 = -3 */
  check_rempow2( -4, 2);  /* -4 % 4 =  0 */
  check_rempow2( -5, 2);  /* -5 % 4 = -1 */
}

Test(rempow2, k_equals_4) {
  check_rempow2(  100, 4);  /* 100 % 16 =  4 */
  check_rempow2( -100, 4);  /*-100 % 16 = -4 */
  check_rempow2(   16, 4);  /*  16 % 16 =  0 */
  check_rempow2(  -16, 4);  /* -16 % 16 =  0 */
  check_rempow2(   15, 4);  /*  15 % 16 = 15 */
  check_rempow2(  -15, 4);  /* -15 % 16 =-15 */
}

Test(rempow2, consistent_with_divpow2) {
  /* r = n - q * 2^k must hold: remainder + quotient*divisor == dividend */
  int32_t ns[] = { 0, 1, -1, 7, -7, 100, -100, 0x7FFFFFFF, (int32_t)0x80000000 };
  int ks[] = { 1, 2, 3, 4, 8 };
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++) {
    for (size_t j = 0; j < sizeof ks / sizeof ks[0]; j++) {
      int32_t n = ns[i], k = ks[j];
      int32_t q = divpow2_asm(n, k);
      int32_t r = rempow2_asm(n, k);
      cr_assert_eq(
        q * (int32_t)(1u << k) + r,
        n,
        "q*2^k + r != n for n=%d k=%d", n, k
      );
    }
  }
}

Test(rempow2, agrees_with_c_reference) {
  int32_t ns[] = { 0, 1, -1, 7, -7, 100, -100, 0x7FFFFFFF, (int32_t)0x80000000 };
  int ks[] = { 1, 2, 3, 4, 8, 16 };
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++)
    for (size_t j = 0; j < sizeof ks / sizeof ks[0]; j++)
      check_rempow2(ns[i], ks[j]);
}

#else
Test(rempow2, skipped) { cr_skip("ARM AArch64 only"); }
#endif

#if defined(__aarch64__)

/*
  Division by 3 using the instruction sequence from
  Hacker's Delight section 10-3. Implements r = n / 3 (truncating).

  li    M, 0x55555556 -- load magic number, (2**32+2)/3
  mulhs q, M, n       -- q = floor(M*n/2**32)
  shri  t, n, 31      -- add 1 to q if
  add   q, q, t       -- n is negative
  muli  t, q, 3       -- compute remainder from
  sub   r, n, t       -- r = n - q*3
 */
static void divrem3_asm(int32_t n, int32_t *q_out, int32_t *r_out) {
  int32_t q, r;
  __asm__ volatile (
    /* M = 0x55555556 -- magic number for division by 3 */
    "mov   w2, #0x5556            \n"
    "movk  w2, #0x5555, lsl #16   \n"  
    /* q = mulhs(M, n) -- signed multiply, take high 32 bits */
    "smull x3, w2, %w[n]          \n"  /* x3 = M * n (64-bit signed) */
    "asr   x3, x3, #32            \n"  /* x3 = high 32 bits          */
    /* t = n >> 31 -- 1 if n negative, else 0 */
    "lsr   w4, %w[n], #31         \n"
    /* q = q + t -- adjust for negative n */
    "add   w3, w3, w4             \n" 
    /* t = q * 3 -- compute q*3 using shift and add */
    "add   w4, w3, w3, lsl #1     \n"  /* w4 = q + q*2 = q*3         */
    /* r = n - t */
    "sub   %w[r], %w[n], w4       \n"
    "mov   %w[q], w3              \n"
    : [q] "=r" (q),
      [r] "=r" (r)
    : [n] "r"  (n)
    : "w2", "w3", "w4", "x3"
  );
  *q_out = q;
  *r_out = r;
}

static void check_divrem3(int32_t n) {
  int32_t q, r;
  divrem3_asm(n, &q, &r);
  cr_assert_eq(
    q,
    n / 3,
    "quotient wrong: divrem3(%d) got q=%d want %d", n, q, n / 3
  );
  cr_assert_eq(
    r,
    n % 3,
    "remainder wrong: divrem3(%d) got r=%d want %d", n, r, n % 3
  );
  cr_assert_eq(
    q * 3 + r,
    n,
    "invariant q*3+r==n failed for n=%d", n
  );
}

Test(divrem3, basic_positive) {
  check_divrem3(0);
  check_divrem3(1);
  check_divrem3(2);
  check_divrem3(3);
  check_divrem3(4);
  check_divrem3(6);
  check_divrem3(7);
  check_divrem3(100);
}

Test(divrem3, basic_negative) {
  check_divrem3(-1);
  check_divrem3(-2);
  check_divrem3(-3);
  check_divrem3(-4);
  check_divrem3(-6);
  check_divrem3(-7);
  check_divrem3(-100);
}

Test(divrem3, boundary_values) {
  check_divrem3(0x7FFFFFFF);   /* INT_MAX */
  check_divrem3(0x7FFFFFFE);
  check_divrem3(0x7FFFFFFD);
  check_divrem3((int32_t)0x80000000);  /* INT_MIN */
  check_divrem3((int32_t)0x80000001);
  check_divrem3((int32_t)0x80000002);
}

Test(divrem3, multiples_of_3) {
  /* remainder must be zero for exact multiples */
  int32_t cases[] = { 3, 6, 9, 12, -3, -6, -9, -12, 300, -300, 0x7FFFFFFE };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    if (cases[i] % 3 != 0) continue;
    int32_t q, r;
    divrem3_asm(cases[i], &q, &r);
    cr_assert_eq(
      r, 0,
      "remainder nonzero for multiple of 3: n=%d r=%d", cases[i], r
    );
  }
}

Test(divrem3, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (int32_t n = -10000; n <= 10000; n++) { check_divrem3(n); }
}

#else
Test(divrem3, skipped) { cr_skip("ARM AArch64 only"); }
#endif

#if defined(__aarch64__)

/*
  Division by 5 using the instruction sequence from
  Hacker's Delight section 10-3. Implements r = n / 5 (truncating).

  li    M, 0x66666667 -- load magic number, (2**33+3)/5
  mulhs q, M, n       -- q = floor(M*n/2**32)
  shrsi q, q, 1 
  shri  t, n, 31      -- add 1 to q if
  add   q, q, t       -- n is negative
  muli  t, q, 5       -- compute remainder from
  sub   r, n, t       -- r = n - q*5
 */
static void divrem5_asm(int32_t n, int32_t *q_out, int32_t *r_out) {
  int32_t q, r;
  __asm__ volatile (
    /* M = 0x66666667 -- magic number for division by 5 */
    "mov   w2, #0x6667            \n"
    "movk  w2, #0x6666, lsl #16   \n"  
    /* q = mulhs(M, n) -- signed multiply, take high 32 bits */
    "smull x3, w2, %w[n]          \n"  /* x3 = M * n (64-bit signed) */
    "asr   x3, x3, #32            \n"  /* x3 = high 32 bits          */
    /* shrsi q, q, 1  */
    "asr   w3, w3, #1             \n"  /* q = q >> 1                 */
    /* t = n >> 31 -- 1 if n negative, else 0 */
    "lsr   w4, %w[n], #31         \n"
    /* q = q + t -- adjust for negative n */
    "add   w3, w3, w4             \n" 
    /* t = q * 5 -- compute q*5 using shift and add */
    "add   w4, w3, w3, lsl #2     \n"  /* w4 = q + q*4 = q*5         */
    /* r = n - t */
    "sub   %w[r], %w[n], w4       \n"
    "mov   %w[q], w3              \n"
    : [q] "=r" (q),
      [r] "=r" (r)
    : [n] "r"  (n)
    : "w2", "w3", "w4", "x3"
  );
  *q_out = q;
  *r_out = r;
}

static void check_divrem5(int32_t n) {
  int32_t q, r;
  divrem5_asm(n, &q, &r);
  cr_assert_eq(
    q,
    n / 5,
    "quotient wrong: divrem5(%d) got q=%d want %d", n, q, n / 5
  );
  cr_assert_eq(
    r,
    n % 5,
    "remainder wrong: divrem5(%d) got r=%d want %d", n, r, n % 5
  );
  cr_assert_eq(
    q * 5 + r,
    n,
    "invariant q*5+r==n failed for n=%d", n
  );
}

Test(divrem5, basic_positive) {
  check_divrem5(0);
  check_divrem5(1);
  check_divrem5(2);
  check_divrem5(3);
  check_divrem5(4);
  check_divrem5(5);
  check_divrem5(6);
  check_divrem5(7);
  check_divrem5(100);
}

Test(divrem5, basic_negative) {
  check_divrem5(-1);
  check_divrem5(-2);
  check_divrem5(-3);
  check_divrem5(-4);
  check_divrem5(-5);
  check_divrem5(-6);
  check_divrem5(-7);
  check_divrem5(-100);
}

Test(divrem5, boundary_values) {
  check_divrem5(0x7FFFFFFF);   /* INT_MAX */
  check_divrem5(0x7FFFFFFE);
  check_divrem5(0x7FFFFFFD);
  check_divrem5((int32_t)0x80000000);  /* INT_MIN */
  check_divrem5((int32_t)0x80000001);
  check_divrem5((int32_t)0x80000002);
}

Test(divrem5, multiples_of_5) {
  /* remainder must be zero for exact multiples */
  int32_t cases[] = { 5, 10, 15, 20, -5, -10, -15, -20, 300, -300, 0x7FFFFFFE };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    if (cases[i] % 5 != 0) continue;
    int32_t q, r;
    divrem5_asm(cases[i], &q, &r);
    cr_assert_eq(
      r, 0,
      "remainder nonzero for multiple of 5: n=%d r=%d", cases[i], r
    );
  }
}

Test(divrem5, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (int32_t n = -10000; n <= 10000; n++) { check_divrem5(n); }
}

#else
Test(divrem5, skipped) { cr_skip("ARM AArch64 only"); }
#endif

#if defined(__aarch64__)

/*
  Division by 7 using the instruction sequence from
  Hacker's Delight section 10-3. Implements r = n / 7 (truncating).

  li    M, 0x92492493 -- load magic number, (2**34+5)/7 - 2**32
  mulhs q, M, n       -- q = floor(M*n/2**32)
  add   q, q, n       -- q = floor(M*n/2**32) + n
  shrsi q, q, 2       -- q = floor(q/4) 
  shri  t, n, 31      -- add 1 to q if
  add   q, q, t       -- n is negative
  muli  t, q, 7       -- compute remainder from
  sub   r, n, t       -- r = n - q*7
 */
static void divrem7_asm(int32_t n, int32_t *q_out, int32_t *r_out) {
  int32_t q, r;
  __asm__ volatile (
    /* M = 0x92492493 -- magic number for division by 7 */
    "mov   w2, #0x2493            \n"
    "movk  w2, #0x9249, lsl #16   \n"  
    /* q = mulhs(M, n) -- signed multiply, take high 32 bits */
    "smull x3, w2, %w[n]          \n"  /* x3 = M * n (64-bit signed) */
    "asr   x3, x3, #32            \n"  /* x3 = high 32 bits          */
    /* add q, q, n -- q = floor(M*n/2**32) + n */
    "add   w3, w3, %w[n]          \n"
    /* shrsi q, q, 2  */
    "asr   w3, w3, #2             \n"  /* q = q >> 2                 */
    /* t = n >> 31 -- 1 if n negative, else 0 */
    "lsr   w4, %w[n], #31         \n"
    /* q = q + t -- adjust for negative n */
    "add   w3, w3, w4             \n" 
    /* t = q * 7 -- compute q*7 using shift and add */
    "lsl   w4, w3, #3             \n"  /* w4 = q * 8   */
    "sub   w4, w4, w3             \n"  /* w4 = q*8 - q = q*7 */
    /* r = n - t */
    "sub   %w[r], %w[n], w4       \n"
    "mov   %w[q], w3              \n"
    : [q] "=r" (q),
      [r] "=r" (r)
    : [n] "r"  (n)
    : "w2", "w3", "w4", "x3"
  );
  *q_out = q;
  *r_out = r;
}

static void check_divrem7(int32_t n) {
  int32_t q, r;
  divrem7_asm(n, &q, &r);
  cr_assert_eq(
    q,
    n / 7,
    "quotient wrong: divrem7(%d) got q=%d want %d", n, q, n / 7
  );
  cr_assert_eq(
    r,
    n % 7,
    "remainder wrong: divrem7(%d) got r=%d want %d", n, r, n % 7
  );
  cr_assert_eq(
    q * 7 + r,
    n,
    "invariant q*7+r==n failed for n=%d", n
  );
}

Test(divrem7, basic_positive) {
  check_divrem7(0);
  check_divrem7(1);
  check_divrem7(2);
  check_divrem7(3);
  check_divrem7(4);
  check_divrem7(5);
  check_divrem7(6);
  check_divrem7(7);
  check_divrem7(100);
}

Test(divrem7, basic_negative) {
  check_divrem7(-1);
  check_divrem7(-2);
  check_divrem7(-3);
  check_divrem7(-4);
  check_divrem7(-5);
  check_divrem7(-6);
  check_divrem7(-7);
  check_divrem7(-100);
}

Test(divrem7, boundary_values) {
  check_divrem7(0x7FFFFFFF);   /* INT_MAX */
  check_divrem7(0x7FFFFFFE);
  check_divrem7(0x7FFFFFFD);
  check_divrem7((int32_t)0x80000000);  /* INT_MIN */
  check_divrem7((int32_t)0x80000001);
  check_divrem7((int32_t)0x80000002);
}

Test(divrem7, multiples_of_7) {
  /* remainder must be zero for exact multiples */
  int32_t cases[] = { 7, 14, 21, 28, -7, -14, -21, -28, 300, -300, 0x7FFFFFFE };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    if (cases[i] % 7 != 0) continue;
    int32_t q, r;
    divrem7_asm(cases[i], &q, &r);
    cr_assert_eq(
      r, 0,
      "remainder nonzero for multiple of 7: n=%d r=%d", cases[i], r
    );
  }
}

Test(divrem7, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (int32_t n = -10000; n <= 10000; n++) { check_divrem7(n); }
}

#else
Test(divrem7, skipped) { cr_skip("ARM AArch64 only"); }
#endif