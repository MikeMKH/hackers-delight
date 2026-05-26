#include <criterion/criterion.h>
#include <stdlib.h>
#include <stdint.h>

Test(examples, addition) {
  int32_t x = 42;
  int32_t y = 58;
  int32_t z = x + y;
  cr_assert_eq(z, 100);
}