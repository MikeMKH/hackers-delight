#include <criterion/criterion.h>
#include <stdlib.h>

Test(counting_bits, divide_and_conquer) {
  uint8_t x = 0b01011000;
  uint8_t e = 3;
  
  x = (x & 0x55) + ((x >> 1) & 0x55);
  x = (x & 0x33) + ((x >> 2) & 0x33);
  x = (x & 0x0F) + ((x >> 4) & 0x0F);
  cr_assert_eq(x, e);
}