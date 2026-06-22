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