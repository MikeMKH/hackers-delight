#include <criterion/criterion.h>

Test(init, one_eq_one) {
  cr_assert_eq(1, 1);
}