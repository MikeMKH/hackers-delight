#include <criterion/criterion.h>
#include <stdlib.h>
#include <stdbool.h> 

bool within_bounds(uint8_t x, uint8_t min, uint8_t max) {
  return (uint8_t)(x - min) <= (uint8_t)(max - min);
}

Test(within_bounds, inside_values) {
  cr_assert(within_bounds(  1, 1, 10), "at min");
  cr_assert(within_bounds(  8, 8, 11), "at min");
  cr_assert(within_bounds(  5, 1, 10), "middle");
  cr_assert(within_bounds( 12, 7, 17), "middle");
  cr_assert(within_bounds( 10, 1, 10), "at max");
  cr_assert(within_bounds(  2, 1,  2), "at max");
}

Test(within_bounds, outside_values) {
  cr_assert(!within_bounds(  0, 1, 100), "below min");
  cr_assert(!within_bounds(  8, 9, 255), "below min");
  cr_assert(!within_bounds(-42, 1, 100), "negative, below min");
  cr_assert(!within_bounds(-77, 9,  20), "negative, below min");
  cr_assert(!within_bounds( 11, 1,  10), "above max");
  cr_assert(!within_bounds(  1, 0,   0), "above max");
  cr_assert(!within_bounds(127, 1,  10), "well above max");
  cr_assert(!within_bounds(255, 1,  10), "well above max");
}