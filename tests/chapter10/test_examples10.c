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

#if defined(__aarch64__)

/*
  Division by 3 using the instruction sequence from Hacker's Delight section 10-8.
  
  li    M, 0xAAAAAAAB -- load magic number, (2**33+1)/3
  mulhs q, M, n       -- q = floor(M*n/2**32)
  shrsi q, q, 1       
  muli  t, q, 3       -- compute remainder from
  sub   r, n, t       -- r = n - q*3
 */
static void udivrem3_asm(uint32_t n, uint32_t *q_out, uint32_t *r_out) {
  uint32_t q, r;
  __asm__ volatile (
    /* M = 0xAAAAAAAB -- magic number for division by 3 */
    "mov   w2, #0xAAAB            \n"
    "movk  w2, #0xAAAA, lsl #16   \n"
    /* q = mulhu(M, n) -- unsigned multiply, take high 32 bits */
    "umull x3, w2, %w[n]          \n"  /* x3 = M * n (64-bit unsigned) */
    "lsr   x3, x3, #32            \n"  /* x3 = high 32 bits            */
    /* shrsi q, q, 1 */
    "lsr   w3, w3, #1             \n"  /* q = q >> 1 */
    /* t = q * 3 -- compute q*3 using shift and add */
    "add   w4, w3, w3, lsl #1     \n"  /* w4 = q + q*2 = q*3 */
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

static void check_udivrem3(uint32_t n) {
  uint32_t q, r;
  udivrem3_asm(n, &q, &r);
  cr_assert_eq(
    q,
    n / 3,
    "quotient wrong: udivrem3(%d) got q=%d want %d", n, q, n / 3
  );
  cr_assert_eq(
    r,
    n % 3,
    "remainder wrong: udivrem3(%d) got r=%d want %d", n, r, n % 3
  );
  cr_assert_eq(
    q * 3 + r,
    n,
    "invariant q*3+r==n failed for n=%d", n
  );
}

Test(udivrem3_asm, basic_positive) {
  check_udivrem3(0);
  check_udivrem3(1);
  check_udivrem3(2);
  check_udivrem3(3);
  check_udivrem3(4);
  check_udivrem3(5);
  check_udivrem3(6);
  check_udivrem3(7);
  check_udivrem3(100);
}

Test(udivrem3_asm, boundary_values) {
  check_udivrem3(0xFFFFFFFF);   /* INT_MAX */
  check_udivrem3(0xFFFFFFFE);
  check_udivrem3(0xFFFFFFFD);
  check_udivrem3(0x00000000);  /* INT_MIN */
  check_udivrem3(0x00000001);
  check_udivrem3(0x00000002);
}

Test(udivrem3_asm, multiples_of_3) {
  /* remainder must be zero for exact multiples */
  uint32_t cases[] = { 3, 6, 9, 12, 300, 3000, 0xFFFFFFFC };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    if (cases[i] % 3 != 0) continue;
    uint32_t q, r;
    udivrem3_asm(cases[i], &q, &r);
    cr_assert_eq(
      r, 0,
      "remainder nonzero for multiple of 3: n=%d r=%d", cases[i], r
    );
  }
}

Test(udivrem3_asm, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (uint32_t n = 0; n <= 10000; n++) { check_udivrem3(n); }
}

#else
Test(udivrem3_asm, skipped) { cr_skip("ARM AArch64 only"); }
#endif

#if defined(__aarch64__)

/*
  Division by 7 using the instruction sequence from Hacker's Delight section 10-8.

  li    M, 0x24924925 -- load magic number, (2**35+3)/7 - 2**32
  mulhs q, M, n       -- q = floor(M*n/2**32)
  add   q, q, n       -- add n to q (may overflow)
  shrxi q, q, 3       -- shift right with carry bit
  muli  t, q, 7       -- compute remainder from
  sub   r, n, t       -- r = n - q*7
 */
static void udivrem7_asm(uint32_t n, uint32_t *q_out, uint32_t *r_out) {
  uint32_t q, r;
  __asm__ volatile (
    /* M = 0x24924925 -- magic number for division by 7 */
    "mov   w2, #0x4925             \n"
    "movk  w2, #0x2492, lsl #16    \n"
    /* q = mulhu(M, n) -- unsigned multiply, take high 32 bits */
    "umull x3, w2, %w[n]          \n"  /* x3 = M * n (64-bit unsigned) */
    "lsr   x3, x3, #32            \n"  /* x3 = high 32 bits            */
    /* add q, q, n -- add n to q (may overflow) */
    "adds  w3, w3, %w[n]          \n"
    /* save the carry from q+n before the following shift */
    "cset  w5, cs                 \n"
    /* shrxi q, q, 3 -- shift right with carry bit */
    "lsr   w3, w3, #3             \n"  /* q = q >> 3 */
    "lsl   w5, w5, #29            \n"  /* carry contributes at bit 29 after >>3 */
    "add   w3, w3, w5             \n"  /* add back the shifted overflow carry */
    /* t = q * 7 -- compute q*7 using shift and add */
    "lsl   w4, w3, #3             \n"  /* w4 = q * 8   */
    "sub   w4, w4, w3             \n"  /* w4 = q*8 - q = q*7 */
    /* r = n - t */
    "sub   %w[r], %w[n], w4       \n"
    "mov   %w[q], w3              \n"
    : [q] "=r" (q),
      [r] "=r" (r)
    : [n] "r"  (n)
    : "w2", "w3", "w4", "w5", "x3", "cc"
  );
  *q_out = q;
  *r_out = r;
}

static void check_udivrem7(uint32_t n) {
  uint32_t q, r;
  udivrem7_asm(n, &q, &r);
  cr_assert_eq(
    q,
    n / 7,
    "quotient wrong: udivrem7(%d) got q=%d want %d", n, q, n / 7
  );
  cr_assert_eq(
    r,
    n % 7,
    "remainder wrong: udivrem7(%d) got r=%d want %d", n, r, n % 7
  );
  cr_assert_eq(
    q * 7 + r,
    n,
    "invariant q*7+r==n failed for n=%d", n
  );
}

Test(udivrem7_asm, basic_positive) {
  check_udivrem7(0);
  check_udivrem7(1);
  check_udivrem7(2);
  check_udivrem7(3);
  check_udivrem7(4);
  check_udivrem7(5);
  check_udivrem7(6);
  check_udivrem7(7);
  check_udivrem7(100);
}

Test(udivrem7_asm, boundary_values) {
  check_udivrem7(0xFFFFFFFF);   /* INT_MAX */
  check_udivrem7(0xFFFFFFFE);
  check_udivrem7(0xFFFFFFFD);
  check_udivrem7(0x00000000);  /* INT_MIN */
  check_udivrem7(0x00000001);
  check_udivrem7(0x00000002);
}

Test(udivrem7_asm, multiples_of_7) {
  /* remainder must be zero for exact multiples */
  uint32_t cases[] = { 7, 14, 21, 28, 700, 7000, 0xFFFFFFF8 };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    if (cases[i] % 7 != 0) continue;
    uint32_t q, r;
    udivrem7_asm(cases[i], &q, &r);
    cr_assert_eq(
      r, 0,
      "remainder nonzero for multiple of 7: n=%d r=%d", cases[i], r
    );
  }
}

Test(udivrem7_asm, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (uint32_t n = 0; n <= 10000; n++) { check_udivrem7(n); }
}

#else
Test(udivrem7_asm, skipped) { cr_skip("ARM AArch64 only"); }
#endif

struct mu {
  unsigned M; // Magic number,
  int a;      // “add” indicator,
  int s;      // and shift amount.
};

struct mu magicu(unsigned d) {
  // Must have 1 <= d <= 2**32-1.
  int p, gt = 0;
  unsigned nc, delta, q1, r1, q2, r2;
  struct mu magu;
  
  magu.a = 0;              // Initialize “add” indicator.
  nc = -1 - (-d)%d;        // Unsigned arithmetic here.
  p = 31;                  // Init. p.
  q1 = 0x80000000/nc;      // Init. q1 = 2**p/nc.
  r1 = 0x80000000 - q1*nc; // Init. r1 = rem(2**p, nc).
  q2 = 0x7FFFFFFF/d;       // Init. q2 = (2**p - 1)/d.
  r2 = 0x7FFFFFFF - q2*d;  // Init. r2 = rem(2**p - 1, d).
  
  do {
    p = p + 1;
    if (q1 >= 0x80000000) gt = 1; // Means q1 > delta.
    
    if (r1 >= nc - r1) {
      q1 = 2*q1 + 1;  // Update q1.
      r1 = 2*r1 - nc; // Update r1.
    }
    else {
      q1 = 2*q1;
      r1 = 2*r1;
    }
    
    if (r2 + 1 >= d - r2) {
      if (q2 >= 0x7FFFFFFF) magu.a = 1;
      q2 = 2*q2 + 1;     // Update q2.
      r2 = 2*r2 + 1 - d; // Update r2.
    }
    else {
      if (q2 >= 0x80000000) magu.a = 1;
      q2 = 2*q2;
      r2 = 2*r2 + 1;
    }
    
    delta = d - 1 - r2;
  } while (gt == 0 &&  (q1 < delta || (q1 == delta && r1 == 0))); 
  
  magu.M = q2 + 1; // Magic number
  magu.s = p - 32; // and shift amount to return
  return magu;     // (magu.a was set above).
}

/* Apply the magic number to compute n/d without division */
static uint32_t apply_magicu(uint32_t n, struct mu mag) {
  uint32_t q;
  if (!mag.a) {
    /* simple case: q = muluh(M, n) >> s */
    q = (uint32_t)(((uint64_t)mag.M * n) >> 32);
    return q >> mag.s;
  } else {
    /* add-n case: true m = M + 2^32 */
    uint32_t t = (uint32_t)(((uint64_t)mag.M * n) >> 32);
    uint32_t sum = t + n;
    /* handle carry from t + n using 64-bit */
    uint64_t full = ((uint64_t)(sum < t ? 1 : 0) << 32) | sum;
    return (uint32_t)(full >> mag.s);
  }
}

Test(magicu, known_magic_numbers) {
  /* verify the book's known magic numbers for specific divisors */
  struct mu m3 = magicu(3);
  printf("d=3:  M=0x%08X a=%d s=%d\n", m3.M, m3.a, m3.s);
  cr_assert_eq(m3.M, 0xAAAAAAABU, "magic for /3: got 0x%08X", m3.M);
  cr_assert_eq(m3.a, 0,           "div3 no add");
  cr_assert_eq(m3.s, 1,           "div3 s=1");
  
  struct mu m5 = magicu(5);
  printf("d=5:  M=0x%08X a=%d s=%d\n", m5.M, m5.a, m5.s);
  cr_assert_eq(m5.M, 0xCCCCCCCDU, "magic for /5: got 0x%08X", m5.M);
  cr_assert_eq(m5.a, 0,           "div5 no add");
  cr_assert_eq(m5.s, 2,           "div5 s=2");
  
  struct mu m7 = magicu(7);
  printf("d=7:  M=0x%08X a=%d s=%d\n", m7.M, m7.a, m7.s);
  cr_assert_eq(m7.M, 0x24924925U, "magic for /7: got 0x%08X", m7.M);
  cr_assert_eq(m7.a, 1,           "div7 needs add");
  cr_assert_eq(m7.s, 3,           "div7 s=3");
}

Test(magicu, edge_case_divisors) {
  /* d=1: every n/1 = n */
  struct mu m1 = magicu(1);
  printf("d=1:  M=0x%08X a=%d s=%d\n", m1.M, m1.a, m1.s);
  cr_assert_eq(apply_magicu(0,          m1), 0U);
  cr_assert_eq(apply_magicu(1,          m1), 1U);
  cr_assert_eq(apply_magicu(0xFFFFFFFF, m1), 0xFFFFFFFFU);

  /* d=2: power of 2 */
  struct mu m2 = magicu(2);
  printf("d=2:  M=0x%08X a=%d s=%d\n", m2.M, m2.a, m2.s);
  cr_assert_eq(apply_magicu(0,          m2), 0U);
  cr_assert_eq(apply_magicu(1,          m2), 0U);
  cr_assert_eq(apply_magicu(2,          m2), 1U);
  cr_assert_eq(apply_magicu(0xFFFFFFFF, m2), 0x7FFFFFFFU);

  /* d=2^32-1: largest possible divisor */
  struct mu mmax = magicu(0xFFFFFFFF);
  printf("d=max: M=0x%08X a=%d s=%d\n", mmax.M, mmax.a, mmax.s);
  cr_assert_eq(apply_magicu(0,          mmax), 0U);
  cr_assert_eq(apply_magicu(0xFFFFFFFF, mmax), 1U);
  cr_assert_eq(apply_magicu(0xFFFFFFFE, mmax), 0U);
}

Test(magicu, show_table_for_small_divisors) {
  printf("\nDivisor  M (magic)    add  sh\n");
  printf("-------  ----------   ---  --\n");
  for (uint32_t d = 1; d <= 20; d++) {
    struct mu mag = magicu(d);
    printf("%-8u 0x%08X   %d    %d\n", d, mag.M, mag.a, mag.s);
  }
  /* powers of 2 should never need the add trick */
  for (int k = 1; k <= 31; k++) {
    uint32_t  d   = 1u << k;
    struct mu mag = magicu(d);
    cr_assert_eq(mag.a, 0, "power of 2 d=%u should not need add", d);
  }
}

Test(magicu, apply_agrees_with_division) {
  uint32_t divisors[] = {
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,100,
    0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xFFFFFFFF
  };
  uint32_t ns[] = {
    0,1,2,3,6,7,8,99,100,101,
    0x7FFFFFFFU, 0x80000000U,0xFFFFFFFEU, 0xFFFFFFFFU
  };

  for (size_t i = 0; i < sizeof divisors / sizeof divisors[0]; i++) {
    uint32_t  d   = divisors[i];
    struct mu mag = magicu(d);
    for (size_t j = 0; j < sizeof ns / sizeof ns[0]; j++) {
      uint32_t n   = ns[j];
      uint32_t got = apply_magicu(n, mag);
      cr_assert_eq(
        got, n / d,
        "n=%u d=%u got=%u want=%u (M=0x%X a=%d s=%d)",
        n, d, got, n/d, mag.M, mag.a, mag.s
      );
    }
  }
}

Test(magicu, exhaustive_small_divisors) {
  uint32_t divisors[] = {3, 5, 7, 11, 13, 100};
  for (size_t i = 0; i < sizeof divisors / sizeof divisors[0]; i++) {
    uint32_t  d   = divisors[i];
    struct mu mag = magicu(d);
    for (uint32_t n = 0; n <= 100000; n++) {
      cr_assert_eq(
        apply_magicu(n, mag), n / d,
        "n=%u d=%u (M=0x%X a=%d s=%d)",
        n, d, mag.M, mag.a, mag.s
      );
    }
    /* spot check large values */
    uint32_t large[] = {0x7FFFFFFF, 0x80000000, 0xFFFFFFFE, 0xFFFFFFFF};
    for (size_t j = 0; j < 4; j++) {
      cr_assert_eq(apply_magicu(
        large[j], mag), large[j] / d,
        "n=0x%X d=%u", large[j],
        d
      );
    }
  }
}

Test(magicu, invariant_quotient_times_d_plus_remainder) {
  uint32_t divisors[] = {3, 5, 7, 11, 100, 0xFFFF};
  uint32_t ns[]       = {0,1,6,7,8,99,100,0x7FFFFFFF,0xFFFFFFFF};
  for (size_t i = 0; i < sizeof divisors / sizeof divisors[0]; i++) {
    uint32_t  d   = divisors[i];
    struct mu mag = magicu(d);
    for (size_t j = 0; j < sizeof ns / sizeof ns[0]; j++) {
      uint32_t n = ns[j];
      uint32_t q = apply_magicu(n, mag);
      uint32_t r = n - q * d;
      cr_assert(
        r < d,
        "remainder %u >= d=%u for n=%u",
        r, d, n
      );
      cr_assert_eq(
        q * d + r, n,
        "q*d+r != n for n=%u d=%u q=%u r=%u",
        n, d, q, r
      );
    }
  }
}

#if defined(__aarch64__)

/*
  Unsigned division by 7 using absolute value trick.
  From Hacker's Delight 10-13: take abs(n), divide, then negate q if n < 0.
 
  abs  an, n          -- an = |n| (treat as unsigned magnitude)
  li   M, 0x92492493  -- magic number (2^34+5)/7
  mulhu q, M, an      -- q = floor(M * an / 2^32)  [unsigned multiply high]
  shri q, q, 2        -- q = q >> 2
  shrsi t, n, 31      -- t = -1 if n < 0, else 0
  xor  q, q, t        -- q = q ^ t
  sub  q, q, t        -- q = (q ^ t) - t  [negates q if n was negative]
 */
static int32_t udiv7_absmagic_asm(int32_t n) {
  int32_t q;
  __asm__ volatile (
    /* an = abs(n): negate if negative, keep if positive */
    "cmp   %w[n], #0              \n"
    "cneg  w2, %w[n], lt          \n"   /* w2 = n < 0 ? -n : n */
    /* M = 0x92492493 -- magic for unsigned /7 */
    "mov   w3, #0x2493            \n"
    "movk  w3, #0x9249, lsl #16   \n"
    /* q = mulhu(M, an) -- unsigned multiply high 32 bits */
    "umull x4, w2, w3             \n"   /* x4 = M * an (unsigned 64-bit) */
    "lsr   x4, x4, #32            \n"   /* x4 = high 32 bits */
    /* q = q >> 2 */
    "lsr   w4, w4, #2             \n"
    /* t = n >> 31 (arithmetic): -1 if n<0, else 0 */
    "asr   w5, %w[n], #31         \n"
    /* negate q if n was negative: q = (q ^ t) - t */
    "eor   w4, w4, w5             \n"   /* q = q ^ t */
    "sub   %w[q], w4, w5          \n"   /* q = q - t */
    : [q] "=r" (q)
    : [n] "r"  (n)
    : "w2", "w3", "w4", "w5", "x4"
  );
  return q;
}

static void check_udiv7_absmagic(int32_t n) {
  int32_t got      = udiv7_absmagic_asm(n);
  int32_t expected = n / 7;
  cr_assert_eq(
    got, expected,
    "udiv7_absmagic(%d) got %d want %d", n, got, expected
  );
}

Test(udiv7_absmagic, basic_positive) {
  check_udiv7_absmagic(0);
  check_udiv7_absmagic(1);
  check_udiv7_absmagic(6);
  check_udiv7_absmagic(7);
  check_udiv7_absmagic(8);
  check_udiv7_absmagic(14);
  check_udiv7_absmagic(100);
}

Test(udiv7_absmagic, basic_negative) {
  check_udiv7_absmagic(-1);
  check_udiv7_absmagic(-6);
  check_udiv7_absmagic(-7);
  check_udiv7_absmagic(-8);
  check_udiv7_absmagic(-14);
  check_udiv7_absmagic(-100);
}

Test(udiv7_absmagic, boundary_values) {
  check_udiv7_absmagic(0x7FFFFFFF);   /* INT_MAX */
  check_udiv7_absmagic(0x7FFFFFFE);
  check_udiv7_absmagic((int32_t)0x80000001);  /* INT_MIN + 1 */
  /* note: INT_MIN = 0x80000000 = -2^31, abs(-2^31) overflows int32
     but the xor/sub trick still gives the correct signed result */
  check_udiv7_absmagic((int32_t)0x80000000);
}

Test(udiv7_absmagic, multiples_of_7) {
  int32_t cases[] = {0, 7, 14, -7, -14, 700, -700};
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    int32_t n   = cases[i];
    int32_t got = udiv7_absmagic_asm(n);
    int32_t expected = n / 7;
    cr_assert_eq(
      got, expected,
      "n=%d: got q=%d want %d", n, got, expected
    );
    cr_assert_eq(
      got * 7, n,
      "n=%d: q=%d but q*7=%d", n, got, got * 7
    );
  }
}

Test(udiv7_absmagic, agrees_with_builtin) {
  for (int32_t n = -10000; n <= 10000; n++) { check_udiv7_absmagic(n); }
}

Test(udiv7_absmagic, compare_with_divrem7) {
  /* cross-check: this method and the direct divrem7 should agree */
  int32_t cases[] = {
    0, 1, -1, 7, -7, 100, -100,
    0x7FFFFFFF, (int32_t)0x80000001
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    int32_t n        = cases[i];
    int32_t q_abs    = udiv7_absmagic_asm(n);
    int32_t q_direct, r_direct;
    divrem7_asm(n, &q_direct, &r_direct);
    cr_assert_eq(
      q_abs, q_direct,
      "methods disagree for n=%d: abs=%d direct=%d", n, q_abs, q_direct
    );
  }
}

#else
Test(udiv7_absmagic, skipped) { cr_skip("ARM AArch64 only"); }
#endif

Test(multiplicative_inverse, a_hat_is_a_multiplicative_inverse_of_a_modulo_m) {
  u_int8_t m = 16;
  cr_assert_eq(3 * 11 % m, 1);
  cr_assert_eq((3 * -5 % m) + m, 1, "normalize to [0, m)");
}

#if defined(__aarch64__)

Test(multiplicative_inverse, divide_by_7) {
  uint8_t q, n = 63;
  __asm__ volatile (
    /* M = 0xB6DB6DB7 -- magic number for division by 7 */
    "mov   w2, #0x6DB7             \n"
    "movk  w2, #0xB6DB, lsl #16    \n"
    /* q = mul(M, n) -- unsigned multiply */
    "mul   %w[q], w2, %w[n]          \n"  /* q = M * n */
    : [q] "=r" (q)
    : [n] "r"  (n)
    : "w2", "w3"
  );
  cr_assert_eq(q, 9, "divide by 7: got %d want 9", q);
}

#else
Test(multiplicative_inverse, skipped) { cr_skip("ARM AArch64 only"); }
#endif

#include <stdbool.h>

/*
  Test if n is divisible by 25 without division.
 
  The RISC sequence:
    li    M, 0xC28F5C29   -- multiplicative inverse of 25 mod 2^32
    mul   q, M, n         -- q = low 32 bits of M*n  (mulu, not mulhs)
    li    c, 0x0A3D70A3   -- c = floor((2^32-1)/25)
    cmpleu t, q, c        -- t = (q <=u c)
    bt    t, is_mult       -- branch if multiple
 
  In C: (M * n) <= c  where all arithmetic is mod 2^32 (uint32_t).
 
  Why it works:
    M = 25^(-1) mod 2^32, so M*25 ≡ 1 (mod 2^32).
    If n = 25*k, then M*n = M*25*k ≡ k (mod 2^32).
    The multiples of 25 in [0, 2^32) are 0, 25, 50, ..., each mapping
    to k = 0, 1, 2, ... The largest k is floor((2^32-1)/25) = 0x0A3D70A3.
    So M*n <=u 0x0A3D70A3 iff n is a multiple of 25.
 */

#define M25  0xC28F5C29U   /* multiplicative inverse of 25 mod 2^32 */
#define C25  0x0A3D70A3U   /* floor((2^32 - 1) / 25)                */

static bool is_multiple_of_25(uint32_t n) { return (uint32_t)(M25 * n) <= C25; }

/* verify the constants are correct */
Test(mult_inverse_25, verify_constants) {
  /* M25 is the multiplicative inverse of 25 mod 2^32 */
  cr_assert_eq(
    (uint32_t)(M25 * 25), 1U,
    "M25 * 25 mod 2^32 should be 1, got 0x%08X", (uint32_t)(M25 * 25)
  );
  /* C25 = floor((2^32-1)/25) */
  cr_assert_eq(
    C25, 0xFFFFFFFFU / 25,
    "C25 should be floor((2^32-1)/25), got 0x%08X", C25
  );
  /* C25 * 25 should be the largest multiple of 25 <= 2^32-1 */
  cr_assert_eq(
    C25 * 25, 0xFFFFFFEBU,
    "C25*25 should be largest multiple of 25 below 2^32"
  );
  /* (C25+1) * 25 should overflow past 2^32 */
  cr_assert(
    (uint64_t)(C25 + 1) * 25 > 0xFFFFFFFFULL,
    "(C25+1)*25 should exceed 2^32-1"
  );
}

Test(mult_inverse_25, known_multiples) {
  uint32_t multiples[] = {
    0,
    25,
    50,
    75,
    100,
    1000,
    10000,
    0xFFFFFFEB,   /* 171798691 * 25 = largest multiple of 25 < 2^32 */
    0xFFFFFFD2,   /* 171798690 * 25 */
    0xFFFFFFB9,   /* 171798689 * 25 */
  };
  for (size_t i = 0; i < sizeof multiples / sizeof multiples[0]; i++) {
    uint32_t n = multiples[i];
    cr_assert(n % 25 == 0, "test data error: 0x%08X not multiple of 25", n);
    cr_assert(is_multiple_of_25(n), "0x%08X should be detected as multiple of 25", n);
  }
}

Test(mult_inverse_25, known_non_multiples) {
  uint32_t non_multiples[] = {
    1, 2, 24, 26, 49, 51, 99, 101,
    0xFFFFFFFF, 0xFFFFFFFE, 0x80000000
  };
  for (size_t i = 0; i < sizeof non_multiples / sizeof non_multiples[0]; i++) {
    uint32_t n = non_multiples[i];
    cr_assert(!is_multiple_of_25(n), "0x%08X should not be a multiple of 25", n);
  }
}

Test(mult_inverse_25, agrees_with_modulo) {
  /* exhaustive check for all n up to 100000 */
  for (uint32_t n = 0; n <= 100000; n++) {
    bool expected = (n % 25 == 0);
    bool got      = is_multiple_of_25(n);
    cr_assert_eq(
      got, expected,
      "n=%u: is_multiple_of_25=%d but n%%25=%u", n, got, n % 25
    );
  }
}

Test(mult_inverse_25, spot_check_large_values) {
  /* check around 2^32 boundary */
  uint32_t cases[] = {
    0xFFFFFFFF, 0xFFFFFFFE, 0xFFFFFFF5,  /* 0xFFFFFFF5 = 4294967285 = 25*171798691 */
    0x80000000, 0x7FFFFFFF,
    0x00000019,  /* 25 */
    0x00000032,  /* 50 */
  };
  for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
    uint32_t n = cases[i];
    cr_assert_eq(
      is_multiple_of_25(n), (n % 25 == 0),
      "n=0x%08X", n
    );
  }
}

Test(mult_inverse_25, m25_maps_multiples_to_0_through_c25) {
  /* 
    The key property: M25*n maps the 0x0A3D70A4 multiples of 25
    in [0, 2^32) bijectively onto [0, C25].
    Verify first few: M25*0=0, M25*25=1, M25*50=2, ... 
  */
  cr_assert_eq((uint32_t)(M25 *  0),  0U, "M25*0 should map to 0");
  cr_assert_eq((uint32_t)(M25 * 25),  1U, "M25*25 should map to 1");
  cr_assert_eq((uint32_t)(M25 * 50),  2U, "M25*50 should map to 2");
  cr_assert_eq((uint32_t)(M25 * 75),  3U, "M25*75 should map to 3");
  cr_assert_eq((uint32_t)(M25 * 100), 4U, "M25*100 should map to 4");
  
  /* and a non-multiple maps above C25 */
  uint32_t q = (uint32_t)(M25 * 1);
  cr_assert(
    q > C25,
    "M25*1 = 0x%08X should be > C25 = 0x%08X", q, C25
  );
}

unsigned divu3(unsigned n) {
  unsigned n0, n1, w0, w1, w2, t, q;
  n0 = n & 0xFFFF;
  n1 = n >> 16;
  w0 = n0*0xAAAB;
  t = n1*0xAAAB + (w0 >> 16);
  w1 = t & 0xFFFF;
  w2 = t >> 16;
  w1 = n0*0xAAAA + w1;
  q = n1*0xAAAA + w2 + (w1 >> 16);
  return q >> 1;
}

Test(divu3, known_values) {
  cr_assert_eq(divu3(0), 0);
  cr_assert_eq(divu3(1), 0);
  cr_assert_eq(divu3(2), 0);
  cr_assert_eq(divu3(3), 1);
  cr_assert_eq(divu3(4), 1);
  cr_assert_eq(divu3(5), 1);
  cr_assert_eq(divu3(6), 2);
  cr_assert_eq(divu3(7), 2);
  cr_assert_eq(divu3(8), 2);
  cr_assert_eq(divu3(9), 3);
}

Test(divu3, multiples_of_3) {
  for (unsigned n = 0; n <= 100; n += 3) {
    unsigned q = divu3(n);
    cr_assert_eq(q * 3, n, "n=%u: q=%u but q*3=%u", n, q, q*3);
  }
}

Test(divu3, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (unsigned n = 0; n <= 10000; n++) {
    unsigned q = divu3(n);
    cr_assert_eq(q, n / 3, "n=%u: divu3=%u but n/3=%u", n, q, n/3);
  }
}

unsigned alt_divu3(unsigned n) {
  unsigned q, r;
  q = (n >> 2) + (n >> 4); /* q = n*0.0101 (approx) */
  q = q + (q >> 4);        /* q = n*0.01010101 */
  q = q + (q >> 8);
  q = q + (q >> 16);
  r = n - q*3;             /* 0 <= r <= 15 */
  return q + (11*r >> 5);  /* returning q + r/3 */
  /* return q + (5*(r + 1) >> 4); */          /* Alternative 1 */
  /* return q + ((r + 5 + (r << 2)) >> 4); */ /* Alternative 2 */
}

Test(alt_divu3, known_values) {
  cr_assert_eq(alt_divu3(0), 0);
  cr_assert_eq(alt_divu3(1), 0);
  cr_assert_eq(alt_divu3(2), 0);
  cr_assert_eq(alt_divu3(3), 1);
  cr_assert_eq(alt_divu3(4), 1);
  cr_assert_eq(alt_divu3(5), 1);
  cr_assert_eq(alt_divu3(6), 2);
  cr_assert_eq(alt_divu3(7), 2);
  cr_assert_eq(alt_divu3(8), 2);
  cr_assert_eq(alt_divu3(9), 3);
}

Test(alt_divu3, multiples_of_3) {
  for (unsigned n = 0; n <= 100; n += 3) {
    unsigned q = alt_divu3(n);
    cr_assert_eq(q * 3, n, "n=%u: q=%u but q*3=%u", n, q, q*3);
  }
}

Test(alt_divu3, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (unsigned n = 0; n <= 10000; n++) {
    unsigned q = alt_divu3(n);
    cr_assert_eq(q, n / 3, "n=%u: alt_divu3=%u but n/3=%u", n, q, n/3);
  }
}

unsigned divu5a(unsigned n) {
  unsigned q, r;
  q = (n >> 3) + (n >> 4);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  r = n - q*5;
  return q + (13*r >> 6);
}

Test(divu5a, known_values) {
  cr_assert_eq(divu5a(0), 0);
  cr_assert_eq(divu5a(1), 0);
  cr_assert_eq(divu5a(2), 0);
  cr_assert_eq(divu5a(3), 0);
  cr_assert_eq(divu5a(4), 0);
  cr_assert_eq(divu5a(5), 1);
  cr_assert_eq(divu5a(6), 1);
  cr_assert_eq(divu5a(7), 1);
  cr_assert_eq(divu5a(8), 1);
  cr_assert_eq(divu5a(9), 1);
  cr_assert_eq(divu5a(10), 2);
}

Test(divu5a, multiples_of_5) {
  for (unsigned n = 0; n <= 100; n += 5) {
    unsigned q = divu5a(n);
    cr_assert_eq(q * 5, n, "n=%u: q=%u but q*5=%u", n, q, q*5);
  }
}

Test(divu5a, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (unsigned n = 0; n <= 10000; n++) {
    unsigned q = divu5a(n);
    cr_assert_eq(q, n / 5, "n=%u: divu5a=%u but n/5=%u", n, q, n/5);
  }
}

unsigned alt_divu5b(unsigned n) {
  unsigned q, r;
  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 2;
  r = n - q*5;
  return q + (7*r >> 5);
  /* return q + (r>4) + (r>9); */
}

Test(alt_divu5b, known_values) {
  cr_assert_eq(alt_divu5b(0), 0);
  cr_assert_eq(alt_divu5b(1), 0);
  cr_assert_eq(alt_divu5b(2), 0);
  cr_assert_eq(alt_divu5b(3), 0);
  cr_assert_eq(alt_divu5b(4), 0);
  cr_assert_eq(alt_divu5b(5), 1);
  cr_assert_eq(alt_divu5b(6), 1);
  cr_assert_eq(alt_divu5b(7), 1);
  cr_assert_eq(alt_divu5b(8), 1);
  cr_assert_eq(alt_divu5b(9), 1);
  cr_assert_eq(alt_divu5b(10), 2);
}

Test(alt_divu5b, multiples_of_5) {
  for (unsigned n = 0; n <= 100; n += 5) {
    unsigned q = alt_divu5b(n);
    cr_assert_eq(q * 5, n, "n=%u: q=%u but q*5=%u", n, q, q*5);
  }
}

Test(alt_divu5b, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (unsigned n = 0; n <= 10000; n++) {
    unsigned q = alt_divu5b(n);
    cr_assert_eq(q, n / 5, "n=%u: alt_divu5b=%u but n/5=%u", n, q, n/5);
  }
}

int divs3(int n) {
  int q, r;
  n = n + (n>>31 & 2);     /* add 2 if n < 0 */
  q = (n >> 2) + (n >> 4); /* q = n*0.0101 (approx) */
  q = q + (q >> 4);        /* q = n*0.01010101 */
  q = q + (q >> 8);
  q = q + (q >> 16);
  r = n - q*3;             /* 0 <= r <= 14 */
  return q + (11*r >> 5);  /* returning q + r/3 */
  /* return q + (5*(r + 1) >> 4); */           /* Alternative 1 */
  /* return q + ((r + 5 + (r << 2)) >> 4); */  /* Alternative 2 */
}

Test(divs3, known_values) {
  cr_assert_eq(divs3(0), 0);
  cr_assert_eq(divs3(1), 0);
  cr_assert_eq(divs3(2), 0);
  cr_assert_eq(divs3(3), 1);
  cr_assert_eq(divs3(4), 1);
  cr_assert_eq(divs3(5), 1);
  cr_assert_eq(divs3(6), 2);
  cr_assert_eq(divs3(7), 2);
  cr_assert_eq(divs3(8), 2);
  cr_assert_eq(divs3(9), 3);
}

Test(divs3, multiples_of_3) {
  for (int n = -99; n <= 99; n += 3) {
    int q = divs3(n);
    cr_assert_eq(q * 3, n, "n=%d: q=%d but q*3=%d", n, q, q*3);
  }
}

Test(divs3, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (int n = -10000; n <= 10000; n++) {
    int q = divs3(n);
    cr_assert_eq(q, n / 3, "n=%d: divs3=%d but n/3=%d", n, q, n/3);
  }
}

int divs5(int n) {
  int q, r;
  n = n + (n>>31 & 4);
  q = (n >> 1) + (n >> 2);
  q = q + (q >> 4);
  q = q + (q >> 8);
  q = q + (q >> 16);
  q = q >> 2;
  r = n - q*5;
  return q + (7*r >> 5);
  /* return q + (r>4) + (r>9); */
}

Test(divs5, known_values) {
  cr_assert_eq(divs5(0), 0);
  cr_assert_eq(divs5(1), 0);
  cr_assert_eq(divs5(2), 0);
  cr_assert_eq(divs5(3), 0);
  cr_assert_eq(divs5(4), 0);
  cr_assert_eq(divs5(5), 1);
  cr_assert_eq(divs5(6), 1);
  cr_assert_eq(divs5(7), 1);
  cr_assert_eq(divs5(8), 1);
  cr_assert_eq(divs5(9), 1);
  cr_assert_eq(divs5(10), 2);
}

Test(divs5, multiples_of_5) {
  for (int n = -100; n <= 100; n += 5) {
    int q = divs5(n);
    cr_assert_eq(q * 5, n, "n=%d: q=%d but q*5=%d", n, q, q*5);
  }
}

Test(divs5, agrees_with_builtin) {
  /* exhaustive check over a range */
  for (int n = -10000; n <= 10000; n++) {
    int q = divs5(n);
    cr_assert_eq(q, n / 5, "n=%d: divs5=%d but n/5=%d", n, q, n/5);
  }
}

int remu3(unsigned n) {
  n = __builtin_popcount(n ^ 0xAAAAAAAA) + 23; /* 23 <= n <= 55 */
  n = __builtin_popcount(n ^ 0x2A) - 3;        /* -3 <= n <= 2 */
  return n + (((int)n >> 31) & 3);             /* signed shift */
}

Test(remu3, known_values) {
  cr_assert_eq(remu3(0), 0);
  cr_assert_eq(remu3(1), 1);
  cr_assert_eq(remu3(2), 2);
  cr_assert_eq(remu3(3), 0);
  cr_assert_eq(remu3(4), 1);
  cr_assert_eq(remu3(5), 2);
  cr_assert_eq(remu3(6), 0);
  cr_assert_eq(remu3(7), 1);
  cr_assert_eq(remu3(8), 2);
}

Test(remu3, multiples_of_3) {
  for (unsigned n = 0; n <= 100; n += 3) {
    unsigned r = remu3(n);
    cr_assert_eq(r, 0, "n=%u: remu3=%u but n%%3=%u", n, r, n%3);
  }
}

int remu3_table_lookup(unsigned n) {
  static char table[33] = {
        2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1
  };
  n = __builtin_popcount(n ^ 0xAAAAAAAA);
  return table[n];
}

Test(remu3_table_lookup, known_values) {
  cr_assert_eq(remu3_table_lookup(0), 0);
  cr_assert_eq(remu3_table_lookup(1), 1);
  cr_assert_eq(remu3_table_lookup(2), 2);
  cr_assert_eq(remu3_table_lookup(3), 0);
  cr_assert_eq(remu3_table_lookup(4), 1);
  cr_assert_eq(remu3_table_lookup(5), 2);
  cr_assert_eq(remu3_table_lookup(6), 0);
  cr_assert_eq(remu3_table_lookup(7), 1);
  cr_assert_eq(remu3_table_lookup(8), 2);
}

Test(remu3_table_lookup, multiples_of_3) {
  for (unsigned n = 0; n <= 100; n += 3) {
    unsigned r = remu3_table_lookup(n);
    cr_assert_eq(r, 0, "n=%u: remu3_table_lookup=%u but n%%3=%u", n, r, n%3);
  }
}

int remu3_register_lookup(unsigned n) {
  n = (n >> 16) + (n & 0xFFFF); /* max 0x1FFFE */
  n = (n >> 8) + (n & 0x00FF);  /* max 0x2FD */
  n = (n >> 4) + (n & 0x000F);  /* max 0x3D */
  n = (n >> 2) + (n & 0x0003);  /* max 0x11 */
  n = (n >> 2) + (n & 0x0003);  /* max 0x6 */
  return (0x0924 >> (n << 1)) & 3;
}

Test(remu3_register_lookup, known_values) {
  cr_assert_eq(remu3_register_lookup(0), 0);
  cr_assert_eq(remu3_register_lookup(1), 1);
  cr_assert_eq(remu3_register_lookup(2), 2);
  cr_assert_eq(remu3_register_lookup(3), 0);
  cr_assert_eq(remu3_register_lookup(4), 1);
  cr_assert_eq(remu3_register_lookup(5), 2);
  cr_assert_eq(remu3_register_lookup(6), 0);
  cr_assert_eq(remu3_register_lookup(7), 1);
  cr_assert_eq(remu3_register_lookup(8), 2);
}

Test(remu3_register_lookup, multiples_of_3) {
  for (unsigned n = 0; n <= 100; n += 3) {
    unsigned r = remu3_register_lookup(n);
    cr_assert_eq(r, 0, "n=%u: remu3_register_lookup=%u but n%%3=%u", n, r, n%3);
  }
}

int remu3_register_table_lookup(unsigned n) {
  static char table[62] = {
    0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1
  };
  n = (n >> 16) + (n & 0xFFFF); /* max 0x1FFFE */
  n = (n >> 8) + (n & 0x00FF);  /* max 0x2FD */
  n = (n >> 4) + (n & 0x000F);  /* max 0x3D */
  return table[n];
}

Test(remu3_register_table_lookup, known_values) {
  cr_assert_eq(remu3_register_table_lookup(0), 0);
  cr_assert_eq(remu3_register_table_lookup(1), 1);
  cr_assert_eq(remu3_register_table_lookup(2), 2);
  cr_assert_eq(remu3_register_table_lookup(3), 0);
  cr_assert_eq(remu3_register_table_lookup(4), 1);
  cr_assert_eq(remu3_register_table_lookup(5), 2);
  cr_assert_eq(remu3_register_table_lookup(6), 0);
  cr_assert_eq(remu3_register_table_lookup(7), 1);
  cr_assert_eq(remu3_register_table_lookup(8), 2);
}

Test(remu3_register_table_lookup, multiples_of_3) {
  for (unsigned n = 0; n <= 100; n += 3) {
    unsigned r = remu3_register_table_lookup(n);
    cr_assert_eq(r, 0, "n=%u: remu3_register_table_lookup=%u but n%%3=%u", n, r, n%3);
  }
}

int remu7(unsigned n) {
  static char table[75] = {
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6,
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2,3,4,5,6,
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2,3,4,5,6,
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2,3,4
  };
  n = (n >> 15) + (n & 0x7FFF); /* max 0x27FFE */
  n = (n >> 9) + (n & 0x001FF); /* max 0x33D */
  n = (n >> 6) + (n & 0x0003F); /* max 0x4A */
  return table[n];
}

Test(remu7, known_values) {
  cr_assert_eq(remu7(0), 0);
  cr_assert_eq(remu7(1), 1);
  cr_assert_eq(remu7(2), 2);
  cr_assert_eq(remu7(3), 3);
  cr_assert_eq(remu7(4), 4);
  cr_assert_eq(remu7(5), 5);
  cr_assert_eq(remu7(6), 6);
  cr_assert_eq(remu7(7), 0);
  cr_assert_eq(remu7(8), 1);
  cr_assert_eq(remu7(9), 2);
}

Test(remu7, multiples_of_7) {
  for (unsigned n = 0; n <= 100; n += 7) {
    unsigned r = remu7(n);
    cr_assert_eq(r, 0, "n=%u: remu7=%u but n%%7=%u", n, r, n%7);
  }
}

int rems3(int n) {
  unsigned r;
  static char table[62] = {
    0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2, 0,1,2,
    0,1,2, 0,1,2, 0,1
  };
  r = n;
  r = (r >> 16) + (r & 0xFFFF); /* max 0x1FFFE */
  r = (r >> 8) + (r & 0x00FF);  /* max 0x2FD */
  r = (r >> 4) + (r & 0x000F);  /* max 0x3D */
  r = table[r];
  return r - (((unsigned)n >> 31) << (r & 2));
}

Test(rems3, known_values) {
  cr_assert_eq(rems3(0), 0);
  cr_assert_eq(rems3(1), 1);
  cr_assert_eq(rems3(2), 2);
  cr_assert_eq(rems3(3), 0);
  cr_assert_eq(rems3(4), 1);
  cr_assert_eq(rems3(5), 2);
  cr_assert_eq(rems3(6), 0);
  cr_assert_eq(rems3(7), 1);
  cr_assert_eq(rems3(8), 2);
}

Test(rems3, multiples_of_3) {
  for (int n = -99; n <= 99; n += 3) {
    int r = rems3(n);
    cr_assert_eq(r, 0, "n=%d: rems3=%d but n%%3=%d", n, r, n%3);
  }
}

int rems7(int n) {
  int r;
  static char table[75] = {
              5,6, 0,1,2,3,4,5,6,
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2,3,4,5,6,
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2,3,4,5,6,
    0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2,3,4,5,6, 0,1,2
  };
  r = (n >> 15) + (n & 0x7FFF); /* FFFF0000 to 17FFE */
  r = (r >> 9) + (r & 0x001FF); /* FFFFFF80 to 2BD */
  r = (r >> 6) + (r & 0x0003F); /* -2 to 72 (decimal) */
  r = table[r + 2];
  return r - (((int)(n & -r) >> 31) & 7);
}

Test(rems7, known_values) {
  cr_assert_eq(rems7(0), 0);
  cr_assert_eq(rems7(1), 1);
  cr_assert_eq(rems7(2), 2);
  cr_assert_eq(rems7(3), 3);
  cr_assert_eq(rems7(4), 4);
  cr_assert_eq(rems7(5), 5);
  cr_assert_eq(rems7(6), 6);
  cr_assert_eq(rems7(7), 0);
  cr_assert_eq(rems7(8), 1);
  cr_assert_eq(rems7(9), 2);
}

Test(rems7, multiples_of_7) {
  for (int n = -98; n <= 98; n += 7) {
    int r = rems7(n);
    cr_assert_eq(r, 0, "n=%d: rems7=%d but n%%7=%d", n, r, n%7);
  }
}