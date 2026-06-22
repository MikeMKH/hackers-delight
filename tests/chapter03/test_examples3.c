#include <criterion/criterion.h>
#include <stdlib.h>

Test(basics, turn_off_rightmost_1_bit) {
  int8_t x = 0b01011000;
  int8_t e = 0b01010000;
  int8_t a = x & (x - 1);
  cr_assert_eq(a, e);
}