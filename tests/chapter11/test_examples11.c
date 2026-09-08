#include <criterion/criterion.h>
#include <stdint.h>

Test(examples11, test_example1) {
  cr_assert_eq(1, 3-2);
}