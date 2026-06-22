#include <criterion/criterion.h>
#include <stdlib.h>

Test(rounding_multiples_of_8, rounding_down) {
  uint8_t x1 = (uint8_t)9;
  uint8_t e1 = (uint8_t)8;
  uint8_t a1 = (x1 & -8);
  cr_assert_eq(a1, e1, "Expected %d, got %d", e1, a1);

  uint8_t x2 = (uint8_t)15;
  uint8_t e2 = (uint8_t)8;
  uint8_t a2 = (x2 & -8);
  cr_assert_eq(a2, e2, "Expected %d, got %d", e2, a2);

  uint8_t x3 = (uint8_t)-37;
  uint8_t e3 = (uint8_t)-40;
  uint8_t a3 = (x3 & -8);
  cr_assert_eq(a3, e3, "Expected %d, got %d", e3, a3);
}

Test(rounding_multiples_of_8, rounding_up) {
  uint8_t x1 = (uint8_t)9;
  uint8_t e1 = (uint8_t)16;
  uint8_t a1 = (x1 + 7) & -8;
  cr_assert_eq(a1, e1, "Expected %d, got %d", e1, a1);
  uint8_t b1 = x1 + (-x1 & 7);
  cr_assert_eq(b1, e1, "Expected %d, got %d", e1, b1);

  uint8_t x2 = (uint8_t)15;
  uint8_t e2 = (uint8_t)16;
  uint8_t a2 = (x2 + 7) & -8;
  cr_assert_eq(a2, e2, "Expected %d, got %d", e2, a2);
  uint8_t b2 = x2 + (-x2 & 7);
  cr_assert_eq(b2, e2, "Expected %d, got %d", e2, b2);

  uint8_t x3 = (uint8_t)17;
  uint8_t e3 = (uint8_t)24;
  uint8_t a3 = (x3 + 7) & -8;
  cr_assert_eq(a3, e3, "Expected %d, got %d", e3, a3);
  uint8_t b3 = x3 + (-x3 & 7);
  cr_assert_eq(b3, e3, "Expected %d, got %d", e3, b3);

  uint8_t x4 = (uint8_t)31;
  uint8_t e4 = (uint8_t)32;
  uint8_t a4 = (x4 + 7) & -8;
  cr_assert_eq(a4, e4, "Expected %d, got %d", e4, a4);
  uint8_t b4 = x4 + (-x4 & 7);
  cr_assert_eq(b4, e4, "Expected %d, got %d", e4, b4);
}

uint32_t flp2(uint32_t x) {
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x - (x >> 1);
}

Test(rounding_next_power_of_2, flp2) {
  uint32_t x1 = 0;
  uint32_t e1 = 0;
  uint32_t a1 = flp2(x1);
  cr_assert_eq(a1, e1, "Expected %d, got %d", e1, a1);

  uint32_t x2 = 1;
  uint32_t e2 = 1;
  uint32_t a2 = flp2(x2);
  cr_assert_eq(a2, e2, "Expected %d, got %d", e2, a2);

  uint32_t x3 = 3;
  uint32_t e3 = 2;
  uint32_t a3 = flp2(x3);
  cr_assert_eq(a3, e3, "Expected %d, got %d", e3, a3);

  uint32_t x4 = 15;
  uint32_t e4 = 8;
  uint32_t a4 = flp2(x4);
  cr_assert_eq(a4, e4, "Expected %d, got %d", e4, a4);
}

uint32_t clp2(uint32_t x) {
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

Test(rounding_next_power_of_2, clp2) {
  uint32_t x1 = 0;
  uint32_t e1 = 0;
  uint32_t a1 = clp2(x1);
  cr_assert_eq(a1, e1, "Expected %d, got %d", e1, a1);

  uint32_t x2 = 1;
  uint32_t e2 = 1;
  uint32_t a2 = clp2(x2);
  cr_assert_eq(a2, e2, "Expected %d, got %d", e2, a2);

  uint32_t x3 = 3;
  uint32_t e3 = 4;
  uint32_t a3 = clp2(x3);
  cr_assert_eq(a3, e3, "Expected %d, got %d", e3, a3);

  uint32_t x4 = 9;
  uint32_t e4 = 16;
  uint32_t a4 = clp2(x4);
  cr_assert_eq(a4, e4, "Expected %d, got %d", e4, a4);
}