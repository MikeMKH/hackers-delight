#include <criterion/criterion.h>
#include <stdint.h>

int32_t isqrt_newton(uint32_t x) {
  uint32_t x1;
  int32_t s, g0, g1;
  
  if (x <= 1) return x;
  s = 1; x1 = x -1;
  if (x1 > 65535) { s += 8; x1 >>= 16; }
  if (x1 > 255)   { s += 4; x1 >>= 8; }
  if (x1 > 15)    { s += 2; x1 >>= 4; }
  if (x1 > 3)     { s += 1; }
  
  g0 = 1 << s; g1 = (g0 + (x >> s)) >> 1;
  while (g1 < g0) {
    g0 = g1;
    g1 = (g0 + (x/g0)) >> 1;
  }
  return g0;
}

Test(isqrt_newton, basic) {
  cr_assert_eq(isqrt_newton(0), 0);
  cr_assert_eq(isqrt_newton(1), 1);
  cr_assert_eq(isqrt_newton(4), 2);
  cr_assert_eq(isqrt_newton(9), 3);
  cr_assert_eq(isqrt_newton(16), 4);
  cr_assert_eq(isqrt_newton(25), 5);
  cr_assert_eq(isqrt_newton(36), 6);
  cr_assert_eq(isqrt_newton(49), 7);
  cr_assert_eq(isqrt_newton(64), 8);
  cr_assert_eq(isqrt_newton(81), 9);
  cr_assert_eq(isqrt_newton(100), 10);
}

Test(isqrt_newton, edge_cases) {
  cr_assert_eq(isqrt_newton(0), 0);
  cr_assert_eq(isqrt_newton(1), 1);
  cr_assert_eq(isqrt_newton(2), 1);
  cr_assert_eq(isqrt_newton(3), 1);
}

Test(isqrt_newton, large_values) {
  cr_assert_eq(isqrt_newton(10000), 100);
  cr_assert_eq(isqrt_newton(1000000), 1000);
}

int32_t isqrt_binary_search(uint32_t x) {
  int32_t s, g0, g1;
  
  if (x <= 4224) {
    if (x <= 24) {
      if (x <= 3) return (x + 3) >> 2;
      else if (x <= 8) return 2;
      else return (x >> 4) + 3;
    } else if (x <= 288) {
      if (x <= 80) s = 3;
      else s = 4;
    } else if (x <= 1088) s = 5;
    else s = 6;
  } else if (x <= 1025 * 1025 -1) {
    if (x <= 257 * 257 - 1) {
      if (x <= 129 * 129 - 1) s = 7;
      else s = 8;
    } else if (x <= 513 * 513 -1) s = 9;
    else s = 10;
  } else if (x <= 4097 * 4097 - 1) {
    if (x <= 2049 * 2049 - 1) s = 11;
    else s = 12;
  } else if (x <= 16385 * 16385 - 1) {
    if (x <= 8193 * 8193 - 1) s = 13;
    else s = 14;
  } else if (x <= 32769 * 32769 - 1) {
    s = 15;
  } else {
    s = 16;
  }
  
  /* same as isqrt_newton */
  g0 = 1 << s; g1 = (g0 + (x >> s)) >> 1;
  while (g1 < g0) {
    g0 = g1;
    g1 = (g0 + (x/g0)) >> 1;
  }
  return g0;
}

Test(isqrt_binary_search, basic) {
  cr_assert_eq(isqrt_binary_search(0), 0);
  cr_assert_eq(isqrt_binary_search(1), 1);
  cr_assert_eq(isqrt_binary_search(4), 2);
  cr_assert_eq(isqrt_binary_search(9), 3);
  cr_assert_eq(isqrt_binary_search(16), 4);
  cr_assert_eq(isqrt_binary_search(25), 5);
  cr_assert_eq(isqrt_binary_search(36), 6);
  cr_assert_eq(isqrt_binary_search(49), 7);
  cr_assert_eq(isqrt_binary_search(64), 8);
  cr_assert_eq(isqrt_binary_search(81), 9);
  cr_assert_eq(isqrt_binary_search(100), 10);
}

Test(isqrt_binary_search, edge_cases) {
  cr_assert_eq(isqrt_binary_search(0), 0);
  cr_assert_eq(isqrt_binary_search(1), 1);
  cr_assert_eq(isqrt_binary_search(2), 1);
  cr_assert_eq(isqrt_binary_search(3), 1);
}

Test(isqrt_binary_search, large_values) {
  cr_assert_eq(isqrt_binary_search(10000), 100);
  cr_assert_eq(isqrt_binary_search(1000000), 1000);
}

int32_t isqrt_simple_binary_search(uint32_t x) {
  uint32_t a, b, m;
  
  a = 1; b = (x >> 5) + 8;
  if (b > 65535) b = 65535;
  do {
    m = (a + b) >> 1;
    if (m * m > x) b = m - 1;
    else           a = m + 1;
  } while (b >= a);
  return a - 1;
}

Test(isqrt_simple_binary_search, basic) {
  cr_assert_eq(isqrt_simple_binary_search(0), 0);
  cr_assert_eq(isqrt_simple_binary_search(1), 1);
  cr_assert_eq(isqrt_simple_binary_search(4), 2);
  cr_assert_eq(isqrt_simple_binary_search(9), 3);
  cr_assert_eq(isqrt_simple_binary_search(16), 4);
  cr_assert_eq(isqrt_simple_binary_search(25), 5);
  cr_assert_eq(isqrt_simple_binary_search(36), 6);
  cr_assert_eq(isqrt_simple_binary_search(49), 7);
  cr_assert_eq(isqrt_simple_binary_search(64), 8);
  cr_assert_eq(isqrt_simple_binary_search(81), 9);
  cr_assert_eq(isqrt_simple_binary_search(100), 10);
}

Test(isqrt_simple_binary_search, edge_cases) {
  cr_assert_eq(isqrt_simple_binary_search(0), 0);
  cr_assert_eq(isqrt_simple_binary_search(1), 1);
  cr_assert_eq(isqrt_simple_binary_search(2), 1);
  cr_assert_eq(isqrt_simple_binary_search(3), 1);
}

Test(isqrt_simple_binary_search, large_values) {
  cr_assert_eq(isqrt_simple_binary_search(10000), 100);
  cr_assert_eq(isqrt_simple_binary_search(1000000), 1000);
}

uint32_t isqrt_hardware_algo(uint32_t x) {
  uint32_t m, y, b;
  
  m = 0x40000000; y = 0;
  while (m != 0) {
    b = y | m;
    y >>= 1;
    if (x >= b) {
      x -= b;
      y |= m;
    }
    m >>= 2;
  }
  return y;
}

Test(isqrt_hardware_algo, basic) {
  cr_assert_eq(isqrt_hardware_algo(0), 0);
  cr_assert_eq(isqrt_hardware_algo(1), 1);
  cr_assert_eq(isqrt_hardware_algo(4), 2);
  cr_assert_eq(isqrt_hardware_algo(9), 3);
  cr_assert_eq(isqrt_hardware_algo(16), 4);
  cr_assert_eq(isqrt_hardware_algo(25), 5);
  cr_assert_eq(isqrt_hardware_algo(36), 6);
  cr_assert_eq(isqrt_hardware_algo(49), 7);
  cr_assert_eq(isqrt_hardware_algo(64), 8);
  cr_assert_eq(isqrt_hardware_algo(81), 9);
  cr_assert_eq(isqrt_hardware_algo(100), 10);
}

Test(isqrt_hardware_algo, edge_cases) {
  cr_assert_eq(isqrt_hardware_algo(0), 0);
  cr_assert_eq(isqrt_hardware_algo(1), 1);
  cr_assert_eq(isqrt_hardware_algo(2), 1);
  cr_assert_eq(isqrt_hardware_algo(3), 1);
}

Test(isqrt_hardware_algo, large_values) {
  cr_assert_eq(isqrt_hardware_algo(10000), 100);
  cr_assert_eq(isqrt_hardware_algo(1000000), 1000);
}