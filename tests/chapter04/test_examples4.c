#include <criterion/criterion.h>
#include <stdlib.h>

Test(rounding_multiples_of_8, rounding_down) {
  uint8_t x1 = (uint8_t)9;
  uint8_t e1 = (uint8_t)8;
  uint8_t a1 = (x1 & -8);
  cr_assert_eq(a1, e1, "Expected %d, got %d", e1, a1);
}