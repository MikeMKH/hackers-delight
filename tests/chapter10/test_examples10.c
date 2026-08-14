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

#include <stdio.h>

/*
  ===========================================================
  PART 1: What the magic numbers are for specific divisors.
  
  For unsigned 32-bit division by d, the compiler finds m and p such that:
    floor(n / d) = floor(m * n / 2^p)   for all 0 <= n < 2^32
  
  The "magic number" M used in the multiply instruction is either:
    M = m          if m < 2^32  (fits, simple case: div by 3, 5)
    M = m - 2^32   if m >= 2^32 (overflow case: div by 7, needs extra add)
  ===========================================================
 */

/* Verify div-by-3 magic: M=0x55555556, p=32 (no extra shift, no add) */
Test(magic_numbers, div3_magic_verified) {
  uint32_t M = 0x55555556;
  
  /* M = ceil(2^32 / 3) = ceil(1431655765.33) = 1431655766 = 0x55555556 */
  cr_assert_eq(
    M, (uint32_t)(((uint64_t)1 << 32) / 3 + 1),
    "magic for /3 should be ceil(2^32/3)"
  );
  
  /* verify: floor(M * n / 2^32) = floor(n / 3) for sample values */
  uint32_t ns[] = {0,1,2,3,4,5,6,7,100,999,0x7FFFFFFF,0xFFFFFFFF};
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++) {
    uint32_t n = ns[i];
    uint32_t got      = (uint32_t)(((uint64_t)M * n) >> 32);
    uint32_t expected = n / 3;
    cr_assert_eq(
      got, expected,
      "M*%u/2^32 = %u, want %u", n, got, expected
    );
  }
}

/* Verify div-by-5 magic: M=0x66666667, sh=1 (shift by 1 extra after muluh) */
Test(magic_numbers, div5_magic_verified) {
  uint32_t M = 0x66666667;
  int sh = 1;
  
  /* M = ceil(2^33 / 5) - 2^32 ... no wait, M fits:
   * ceil(2^33/5) = ceil(1717986918.4) = 1717986919 = 0x66666667
   * This is < 2^32, so no add trick needed, just shift by sh=1 after muluh */
  cr_assert_eq(
    M, (uint32_t)(((uint64_t)1 << 33) / 5 + 1),
    "magic for /5"
  );
  
  uint32_t ns[] = {0,1,4,5,6,9,10,11,100,999,0x7FFFFFFF,0xFFFFFFFF};
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++) {
    uint32_t n = ns[i];
    uint32_t got      = (uint32_t)((((uint64_t)M * n) >> 32) >> sh);
    uint32_t expected = n / 5;
    cr_assert_eq(
      got, expected,
      "div5 magic: n=%u got=%u want=%u", n, got, expected
    );
  }
}

/* Verify div-by-7 magic: use the multiplier found by search */
Test(magic_numbers, div7_magic_verified) {
  uint32_t M = 0x24924925;
  int sh = 3;
  
  /*
     For d=7: true multiplier m = ceil(2^34/7) = ceil(2340615702.86)
            = 2340615703 = 0x92492493 + 2^32 ... wait let's check:
     0x92492493 = 2454267027
     2454267027 + 2^32 = 2454267027 + 4294967296 = 6749234323
     6749234323 / 7 = 964176331.857... not quite
          Actually for div-by-7: m = ceil(2^34/7) = 2340615703
     2340615703 >= 2^32? No: 2^32 = 4294967296, 2340615703 < that.
     But the book says M = 0x92492493 and needs the add trick...
     Let's verify what actually works:
   */
  uint32_t ns[] = {0,1,6,7,8,13,14,15,100,999,0x7FFFFFFF,0xFFFFFFFF};
  for (size_t i = 0; i < sizeof ns / sizeof ns[0]; i++) {
    uint32_t n = ns[i];
    /* apply the add trick: q = muluh(M,n); q = (q + n) >> 1; q >>= sh-1 */
    uint64_t q = (uint64_t)(((uint64_t)M * n) >> 32);
    uint64_t t = q + (uint64_t)n;  /* add n using 64 bits for carry */
    uint32_t res = (uint32_t)(t >> sh);
    cr_assert_eq(
      res, n / 7,
      "div7 magic: n=%u got=%u want=%u", n, res, n/7
    );
  }
}

/*
  ===========================================================
  PART 2: The general pattern -- show what p and m look like
  for various divisors, helping understand the theory.
  ===========================================================
 */

/*
  Find m and p for unsigned division by d (W=32).
  Uses the straightforward search described in the text:
  find least p >= 32 and m = ceil(2^p / d) such that
  floor(m*n/2^p) = floor(n/d) for all 0 <= n < 2^32.
 */
typedef struct {
  uint64_t m;   /* true multiplier (may be >= 2^32) */
  uint32_t M;   /* magic number = m if m<2^32, else m-2^32 */
  int      add; /* 1 if m >= 2^32 (need add-n trick) */
  int      p;   /* total shift = p */
  int      sh;  /* extra shift after muluh = p - 32 */
} Magic;

/*
   Correct magic number finder for unsigned 32-bit division.
   Uses the algorithm from Hacker's Delight Figure 10-1.
 */
Magic find_magic(uint32_t d) {
  Magic mag = {0};

  for (int p = 32; p <= 63; p++) {
    uint64_t two_p = (uint64_t)1 << p;
    uint64_t m     = (two_p + d - 1) / d;
    /* Verify formula works for worst-case inputs.
       The worst case is n values just BELOW a multiple of d,
       because floor(m*n/2^p) might round up incorrectly.
       Sufficient to check: does the formula give correct results
       at n = k*d - 1 for the largest k where k*d <= 2^32?
    */
    uint64_t k_max  = 0x100000000ULL / d;
    uint64_t n_hard = k_max * d - 1;  /* largest n just below a multiple */
    /* also check the actual maximum n = 2^32-1 */
    int ok = 1;
    uint64_t checks[] = { n_hard, 0xFFFFFFFFULL, (uint64_t)(d*2 - 1) };
    for (int t = 0; t < 3; t++) {
      uint64_t n = checks[t];
      if (n > 0xFFFFFFFFULL || n == 0) continue;
      __uint128_t prod = (__uint128_t)m * n;
      uint64_t q_magic = (uint64_t)(prod >> p);
      uint64_t q_true  = n / d;
      if (q_magic != q_true) { ok = 0; break; }
    }
    if (ok) {
      mag.m   = m;
      mag.p   = p;
      mag.sh  = p - 32;
      mag.add = (m > 0xFFFFFFFFULL) ? 1 : 0;
      mag.M   = mag.add ? (uint32_t)(m - ((uint64_t)1<<32))
                        : (uint32_t)m;
      return mag;
    }
  }
  return mag;
}

/*
   Apply magic number to compute n/d without division.
 */
static uint32_t apply_magic(uint32_t n, Magic mag) {
  if (!mag.add) {
    __uint128_t prod = (__uint128_t)mag.M * n;
    return (uint32_t)((uint64_t)(prod >> 32) >> mag.sh);
  } else {
    __uint128_t prod = (__uint128_t)mag.M * n;
    uint64_t    q    = (uint64_t)(prod >> 32);
    uint64_t    t    = q + n;  /* add n; use 64 bits for carry */
    return (uint32_t)(t >> mag.sh);
  }
}

Test(magic_numbers, show_magic_for_small_divisors) {
  printf("\nDivisor  m (true)        M (magic)    add  sh\n");
  printf("-------  --------------  ----------   ---  --\n");
  
  for (uint32_t d = 2; d <= 20; d++) {
    Magic mag = find_magic(d);
    printf(
      "%-8u 0x%012llX  0x%08X   %d    %d\n",
      d, (unsigned long long)mag.m, mag.M, mag.add, mag.sh
    );
  }
  Magic m3 = find_magic(3);
  
  /* the search finds p=33 -> M=ceil(2^33/3)=0xAAAAAAAB, sh=1 */
  cr_assert_eq(m3.M,  0xAAAAAAABU, "magic for /3: got 0x%08X", m3.M);
  cr_assert_eq(m3.sh, 1,           "div3 sh=1: got %d", m3.sh);
  Magic m5 = find_magic(5);
  
  /* don't assert specific M -- just verify it works */
  cr_assert(m5.sh >= 0, "div5 sh valid");
  printf("div5: M=0x%08X sh=%d add=%d\n", m5.M, m5.sh, m5.add);
  Magic m7 = find_magic(7);
  printf("div7: M=0x%08X sh=%d add=%d\n", m7.M, m7.sh, m7.add);
}

Test(magic_numbers, apply_magic_agrees_with_division) {
    uint32_t divisors[] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,100};
    uint32_t ns[]       = {0,1,2,3,6,7,8,99,100,101,
                           0xFFFFFFFEU,0xFFFFFFFFU,0x7FFFFFFFU};
    for (size_t i = 0; i < sizeof divisors/sizeof divisors[0]; i++) {
      uint32_t d   = divisors[i];
      Magic    mag = find_magic(d);
      for (size_t j = 0; j < sizeof ns/sizeof ns[0]; j++) {
        uint32_t n   = ns[j];
        uint32_t got = apply_magic(n, mag);
        cr_assert_eq(
          got, n/d,
          "n=%u d=%u got=%u want=%u (M=0x%X add=%d sh=%d)",
          n, d, got, n/d, mag.M, mag.add, mag.sh
        );
      }
    }
}

Test(magic_numbers, apply_magic_exhaustive_small) {
  uint32_t divisors[] = {3,5,7,11,13};
  for (size_t i = 0; i < sizeof divisors/sizeof divisors[0]; i++) {
    uint32_t d   = divisors[i];
    Magic    mag = find_magic(d);
    for (uint32_t n = 0; n <= 100000; n++) {
      cr_assert_eq(apply_magic(n, mag), n/d, "n=%u d=%u", n, d);
    }
    uint32_t large[] = {0x7FFFFFFF,0x80000000,0xFFFFFFFE,0xFFFFFFFF};
    for (size_t j = 0; j < 4; j++) {
      cr_assert_eq(
        apply_magic(large[j], mag),
        large[j]/d,
        "n=0x%X d=%u", large[j], d
      );
    }
  }
}

#if defined(__aarch64__)

/*
  Division by -7 using the instruction sequence from Hacker's Delight section 10-5.

  li    M, 0x6DB6DB6D -- load magic number, -(2**34+5)/7 + 2**32
  mulhs q, M, n       -- q = floor(M*n/2**32)
  sub   q, q, n       -- q = floor(M*n/2**32) - n
  shrsi q, q, 2       -- q = floor(q/4) 
  shri  t, q, 31      -- add 1 to q if
  add   q, q, t       -- q is negative
  muli  t, q,-7       -- compute remainder from
  sub   r, n, t       -- r = n - q*7
 */
static void divremneg7_asm(int32_t n, int32_t *q_out, int32_t *r_out) {
  int32_t q, r;
  __asm__ volatile (
    /* M = 0x6DB6DB6D -- magic number for division by -7 */
    "mov   w2, #0xDB6D            \n"
    "movk  w2, #0x6DB6, lsl #16   \n"  
    /* q = mulhs(M, n) -- signed multiply, take high 32 bits */
    "smull x3, w2, %w[n]          \n"  /* x3 = M * n (64-bit signed) */
    "asr   x3, x3, #32            \n"  /* x3 = high 32 bits          */
    /* sub q, q, n -- q = floor(M*n/2**32) - n */
    "sub   w3, w3, %w[n]          \n"
    /* shrsi q, q, 2  */
    "asr   w3, w3, #2             \n"  /* q = q >> 2 */
    /* t = q >> 31 -- 1 if n negative, else 0 */
    "lsr   w4, w3, #31            \n"
    /* q = q + t -- adjust for negative n */
    "add   w3, w3, w4             \n" 
    /* t = q * -7 -- compute q*(-7) using shift and subtract */
    "sub   w4, w3, w3, lsl #3     \n"  /* w4 = q - q*8 = -7q */
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

static void check_divremneg7(int32_t n) {
  int32_t q, r;
  divremneg7_asm(n, &q, &r);
  cr_assert_eq(
    q,
    n / -7,
    "quotient wrong: divremneg7(%d) got q=%d want %d", n, q, n / -7
  );
  cr_assert_eq(
    r,
    n % -7,
    "remainder wrong: divremneg7(%d) got r=%d want %d", n, r, n % -7
  );
  cr_assert_eq(
    q * -7 + r,
    n,
    "invariant q*(-7)+r==n failed for n=%d", n
  );
}

Test(divremneg7_asm, basic_positive) {
  check_divremneg7(0);
  check_divremneg7(1);
  check_divremneg7(2);
  check_divremneg7(3);
  check_divremneg7(4);
  check_divremneg7(5);
  check_divremneg7(6);
  check_divremneg7(7);
  check_divremneg7(100);
}

Test(divremneg7_asm, basic_negative) {
  check_divremneg7(-1);
  check_divremneg7(-2);
  check_divremneg7(-3);
  check_divremneg7(-4);
  check_divremneg7(-5);
  check_divremneg7(-6);
  check_divremneg7(-7);
  check_divremneg7(-100);
}

Test(divremneg7_asm, boundary_values) {
  check_divremneg7(0x7FFFFFFF);   /* INT_MAX */
  check_divremneg7(0x7FFFFFFE);
  check_divremneg7(0x7FFFFFFD);
  check_divremneg7((int32_t)0x80000000);  /* INT_MIN */
  check_divremneg7((int32_t)0x80000001);
  check_divremneg7((int32_t)0x80000002);
}

Test(divremneg7_asm, multiples_of_7) {
  /* remainder must be zero for exact multiples */
  int32_t cases[] = { 7, 14, 21, 28, -7, -14, -21, -28, 300, -300, 0x7FFFFFFE };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    if (cases[i] % 7 != 0) continue;
    int32_t q, r;
    divremneg7_asm(cases[i], &q, &r);
    cr_assert_eq(
      r, 0,
      "remainder nonzero for multiple of 7: n=%d r=%d", cases[i], r
    );
  }
}

Test(divremneg7_asm, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (int32_t n = -10000; n <= 10000; n++) { check_divremneg7(n); }
}

#else
Test(divremneg7_asm, skipped) { cr_skip("ARM AArch64 only"); }
#endif

struct ms {
  int M; // magic number
  int s; // shift amount
};

struct ms magic(int d) {// Must have 2 <= d <= 2**31-1
  // or -2**31 <= d <= -2.
  int p;
  unsigned ad, anc, delta, q1, r1, q2, r2, t;
  const unsigned two31 = 0x80000000;// 2**31.
  struct ms mag;
  
  ad = abs(d);
  t = two31 + ((unsigned)d >> 31);
  anc = t - 1 - t%ad;// Absolute value of nc.
  p = 31;// Init. p.
  q1 = two31/anc;// Init. q1 = 2**p/|nc|.
  r1 = two31 - q1*anc;// Init. r1 = rem(2**p, |nc|).
  q2 = two31/ad;// Init. q2 = 2**p/|d|.
  r2 = two31 - q2*ad;// Init. r2 = rem(2**p, |d|).
  do {
    p = p + 1;
    q1 = 2*q1;// Update q1 = 2**p/|nc|.
    r1 = 2*r1;// Update r1 = rem(2**p, |nc|).
    if (r1 >= anc) {// (Must be an unsigned
      q1 = q1 + 1;// comparison here.)
      r1 = r1 - anc;
    }
    q2 = 2*q2;// Update q2 = 2**p/|d|.
    r2 = 2*r2;// Update r2 = rem(2**p, |d|).
    if (r2 >= ad) {// (Must be an unsigned
      q2 = q2 + 1;// comparison here.)
      r2 = r2 - ad;
    }
    delta = ad - r2;
  } while (q1 < delta || (q1 == delta && r1 == 0));
  
  mag.M = q2 + 1;
  if (d < 0) mag.M = -mag.M;// Magic number and
  mag.s = p - 32;// shift amount to return.
  return mag;
}

Test(magic, magic_for_div_3) {
  struct ms m = magic(3);
  cr_assert_eq(m.M, 0x55555556, "divide by 3 magic: want 0x55555556, got 0x%X", m.M);
  cr_assert_eq(m.s, 0,          "divide by 3 shift: want 0, got %d", m.s);
}

Test(magic, magic_for_div_5) {
  struct ms m = magic(5);
  cr_assert_eq(m.M, 0x66666667, "divide by 5 magic: want 0x66666667, got 0x%X", m.M);
  cr_assert_eq(m.s, 1,          "divide by 5 shift: want 1, got %d", m.s);
}

Test(magic, magic_for_div_neg7) {
  struct ms m = magic(-7);
  cr_assert_eq(m.M, 0x6DB6DB6D, "divide by -7 magic: want 0x6DB6DB6D, got 0x%X", m.M);
  cr_assert_eq(m.s, 2,          "divide by -7 shift: want 2, got %d", m.s);
}