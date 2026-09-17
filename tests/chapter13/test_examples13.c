#include <criterion/criterion.h>
#include <stdint.h>

Test(subtraction, known_values) {
  cr_assert_eq(1 - 1, 0);
  cr_assert_eq(2 - 1, 1);
  cr_assert_eq(3 - 1, 2);
  cr_assert_eq(4 - 1, 3);
  cr_assert_eq(5 - 1, 4);
  cr_assert_eq(6 - 1, 5);
  cr_assert_eq(7 - 1, 6);
  cr_assert_eq(8 - 1, 7);
}