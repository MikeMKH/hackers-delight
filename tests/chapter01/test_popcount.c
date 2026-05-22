#include <criterion/criterion.h>
#include "popcount.h"

Test(popcount_asm, zero) {
  cr_assert_eq(popcount_asm(0), 0);
}

Test(popcount_asm, all_ones) {
  cr_assert_eq(popcount_asm(0xFFFFFFFF), 32);
}

Test(popcount_asm, agrees_with_builtin) {
  for (uint32_t x = 0; x < 10000; x++) {
    cr_assert_eq(popcount_asm(x), __builtin_popcount(x));
  }
}