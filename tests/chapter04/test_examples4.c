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

typedef struct { uint8_t lo; uint8_t hi; } Bounds;

Bounds add_bounds(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  /* compute tight bounds on x+y given a<=x<=b, c<=y<=d */
  uint16_t lo = (uint16_t)a + c;
  uint16_t hi = (uint16_t)b + d;
  if (lo <= 255 && hi >= 256) {
    /* overflow case: result spans the entire range */
    return (Bounds){ 0, 255 };
  } else {
    /* no overflow: bounds add normally */
    return (Bounds){ (uint8_t)lo, (uint8_t)hi };
  }
}

Test(add_bounds, no_overflow) {
  /* x in [10,20], y in [5,15]: x+y in [15,35], no overflow */
  uint8_t x = 17, y = 12, a = 10, b = 20, c = 5, d = 15;
  
  Bounds r = add_bounds(a, b, c, d);
  cr_assert_eq(r.lo, 15);
  cr_assert_eq(r.hi, 35);

  /* verify actual sum stays within computed bounds */
  cr_assert(within_bounds((int8_t)(x + y), (int8_t)r.lo, (int8_t)r.hi));
}

Test(add_bounds, spans_full_range) {
    /* x in [10,200], y in [10,200]: a+c=20<=255, b+d=400>=256 -> [0,255] */
    Bounds r = add_bounds(10, 200, 10, 200);
    cr_assert_eq(r.lo,   0);
    cr_assert_eq(r.hi, 255);
}

Test(add_bounds, tight_at_boundary) {
    /* x in [0,127], y in [0,128]: b+d = 255, just fits */
    Bounds r = add_bounds(0, 127, 0, 128);
    cr_assert_eq(r.lo,   0);
    cr_assert_eq(r.hi, 255);
}

bool within_bounds_unsigned(uint8_t x, uint8_t min, uint8_t max) {
    return (uint8_t)(x - min) <= (uint8_t)(max - min);
}

Test(add_bounds, upper_overflow) {
    /* x in [200,240], y in [100,200]: b+d = 440 >= 256, a+c = 300 >= 256 */
    uint8_t x = 220, y = 150, a = 200, b = 240, c = 100, d = 200;
    /* both wrap: lo=(uint8_t)300=44, hi=(uint8_t)440=184 */
    Bounds r = add_bounds(a, b, c, d);
    cr_assert_eq(r.lo,  44);
    cr_assert_eq(r.hi, 184);
    
    /* x+y=(uint8_t)370=114, check 44 <= 114 <= 184 */
    uint8_t sum = (uint8_t)(x + y);
    cr_assert_eq(sum, 114);
    cr_assert(within_bounds_unsigned(sum, r.lo, r.hi));
}
