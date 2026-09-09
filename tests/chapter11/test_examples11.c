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

int32_t icbrt(uint32_t x) {
  int32_t s;
  uint32_t y, b;
  
  y = 0;
  for (s = 30; s >= 0; s -= 3) {
    y *= 2;
    b = (3 * y * (y + 1) + 1) << s;
    if (x >= b) { x -= b; y++; }
  }
  return y;
}

Test(icbrt, basic) {
  cr_assert_eq(icbrt(0), 0);
  cr_assert_eq(icbrt(1), 1);
  cr_assert_eq(icbrt(8), 2);
  cr_assert_eq(icbrt(27), 3);
  cr_assert_eq(icbrt(64), 4);
  cr_assert_eq(icbrt(125), 5);
}

Test(icbrt, edge_cases) {
  cr_assert_eq(icbrt(0), 0);
  cr_assert_eq(icbrt(1), 1);
  cr_assert_eq(icbrt(2), 1);
  cr_assert_eq(icbrt(3), 1);
}

#include <stdio.h>

uint8_t icbrt_u8(uint8_t x) {
  uint8_t y = 0;
  uint8_t b;

  for (int s = 6; s >= 0; s -= 3) {
    y = (uint8_t)(y * 2);
    b = (uint8_t)((3 * y * (y + 1) + 1) << s);
    if (x >= b) {
      x = (uint8_t)(x - b);
      y = (uint8_t)(y + 1);
    }
  }
  return y;
}

uint8_t icbrt_u8_print_trace(uint8_t x) {
  uint8_t y = 0;
  uint8_t b;

  printf("x = %3u  (0b", x);
  for (int bit = 7; bit >= 0; bit--) {
    putchar(((x >> bit) & 1) ? '1' : '0');
  }
  printf(")\n\n");

  printf(" s   y_in    b   x_in  x>=b?  y_out  x_out\n");
  printf("--   ----  -----  ----  -----  -----  -----\n");

  for (int s = 6; s >= 0; s -= 3) {
    uint8_t y_in = y;
    uint8_t x_in = x;

    y = (uint8_t)(y * 2);
    b = (uint8_t)((3 * y * (y + 1) + 1) << s);

    int took = (x >= b);
    if (took) {
      x = (uint8_t)(x - b);
      y = (uint8_t)(y + 1);
    }

    printf("%2d    %3u  %5u   %3u   %s     %3u    %3u\n",
           s, y_in, b, x_in, took ? "yes" : " no", y, x);
  }

  printf("\ncube root = %u, remainder = %u\n", y, x);
  return y;
}

/*
  Reference implementation: largest r such that r^3 <= x. Deliberately
  written differently from icbrt_u8 (linear search, no bit tricks) so
  it can't share a bug with the code under test.
*/
static uint8_t reference_cbrt(uint8_t x) {
  uint8_t r = 0;
  while ((uint32_t)(r + 1) * (r + 1) * (r + 1) <= x) {
    r++;
  }
  return r;
}

Test(icbrt_u8, exhaustive_8bit) {
  for (int x = 0; x <= 255; x++) {
    uint8_t want = reference_cbrt((uint8_t)x);
    uint8_t got = icbrt_u8((uint8_t)x);
    cr_assert_eq(got, want, "x=%d got=%u want=%u", x, got, want);
  }
}

Test(icbrt_u8, known_cubes) {
  cr_assert_eq(icbrt_u8(0), 0);
  cr_assert_eq(icbrt_u8(1), 1);
  cr_assert_eq(icbrt_u8(8), 2);
  cr_assert_eq(icbrt_u8(27), 3);
  cr_assert_eq(icbrt_u8(64), 4);
  cr_assert_eq(icbrt_u8(125), 5);
  cr_assert_eq(icbrt_u8(216), 6);
}

Test(icbrt_u8, edge_cases) {
  cr_assert_eq(icbrt_u8(0), 0);
  cr_assert_eq(icbrt_u8(1), 1);
  cr_assert_eq(icbrt_u8(2), 1);
  cr_assert_eq(icbrt_u8(3), 1);
  cr_assert_eq(icbrt_u8(7), 1);   /* just below 2^3   */
  cr_assert_eq(icbrt_u8(9), 2);   /* just above 2^3   */
  cr_assert_eq(icbrt_u8(255), 6); /* 6^3=216, 7^3=343 */
}

/*
  No assertions, prints the cycle-by-cycle trace to stdout.
*/
Test(icbrt_u8, print_trace_demo) {
  icbrt_u8_print_trace(179);
  printf("\n");
  icbrt_u8_print_trace(255);
}

/*
x = 179  (0b10110011)

 s   y_in    b   x_in  x>=b?  y_out  x_out
--   ----  -----  ----  -----  -----  -----
 6      0     64   179   yes       1    115
 3      1    152   115    no       2    115
 0      2     61   115   yes       5     54

cube root = 5, remainder = 54

x = 255  (0b11111111)

 s   y_in    b   x_in  x>=b?  y_out  x_out
--   ----  -----  ----  -----  -----  -----
 6      0     64   255   yes       1    191
 3      1    152   191   yes       3     39
 0      3    127    39    no       6     39

cube root = 6, remainder = 39
*/