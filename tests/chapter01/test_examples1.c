#include <criterion/criterion.h>
#include <stdlib.h>

Test(examples, addition) {
  int x = 42;
  int y = 58;
  int z = x + y;
  cr_assert_eq(z, 100);
}