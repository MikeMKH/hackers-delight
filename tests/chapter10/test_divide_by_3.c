#include <criterion/criterion.h>
#include <stdint.h>
#include "divide_by_3.h"

static void check_divide(uint8_t dividend) {
  uint8_t remainder;
  uint8_t q = divide_by_3(dividend, &remainder);
  cr_assert_eq(
    q, (uint8_t)(dividend / 3),
    "dividend=%u got q=%u want=%u", dividend, q, dividend / 3
  );
  cr_assert_eq(
    remainder, (uint8_t)(dividend % 3),
    "dividend=%u got r=%u want=%u", dividend, remainder, dividend % 3
  );
}

Test(divide_by_3, exhaustive_8bit) {
  for (int dividend = 0; dividend <= 255; dividend++) {
    check_divide((uint8_t)dividend);
  }
}

Test(divide_by_3, zero) {
  check_divide(0b00000000);
}

Test(divide_by_3, max) {
  check_divide(0b11111111);
}

/* 
  Cross-check individual stages against Table 10-5 directly, so a
  regression in the gate logic itself (not just the driver loop)
  shows up on its own.
*/
Test(divide_by_3, stage_table_10_5) {
  uint8_t y;
  div3_state_t out;

  div3_stage((div3_state_t){0, 0}, 0, &y, &out);
  cr_assert_eq(y, 0); cr_assert_eq(out.r, 0); cr_assert_eq(out.s, 0);

  div3_stage((div3_state_t){0, 0}, 1, &y, &out);
  cr_assert_eq(y, 0); cr_assert_eq(out.r, 0); cr_assert_eq(out.s, 1);

  div3_stage((div3_state_t){0, 1}, 0, &y, &out);
  cr_assert_eq(y, 0); cr_assert_eq(out.r, 1); cr_assert_eq(out.s, 0);

  div3_stage((div3_state_t){0, 1}, 1, &y, &out);
  cr_assert_eq(y, 1); cr_assert_eq(out.r, 0); cr_assert_eq(out.s, 0);

  div3_stage((div3_state_t){1, 0}, 0, &y, &out);
  cr_assert_eq(y, 1); cr_assert_eq(out.r, 0); cr_assert_eq(out.s, 1);

  div3_stage((div3_state_t){1, 0}, 1, &y, &out);
  cr_assert_eq(y, 1); cr_assert_eq(out.r, 1); cr_assert_eq(out.s, 0);
}

/* Trace a known example: 25 / 3 = 8 remainder 1 (25 = 0b00011001). */
Test(divide_by_3, trace_example) {
  div3_state_t trace[8];
  uint8_t remainder;
  uint8_t q = divide_by_3_traced(0b00011001, &remainder, trace);
 
  cr_assert_eq(q, 8, "quotient");
  cr_assert_eq(remainder, 1, "remainder");
}
 
/*
  No assertions -- this test exists purely to print the circuit's
  cycle-by-cycle trace to stdout. Run with --verbose (or
  -j1 --verbose if output ordering matters) to see it.
*/
Test(divide_by_3, print_trace_demo) {
  divide_by_3_print_trace(25);
  printf("\n");
  divide_by_3_print_trace(0b11111111);
}

/*
dividend =  25  (0b00011001)

 i   r_in s_in | x_i || y_i  r_i  s_i
--   ---- ---- | --- || ---  ---  ---
 7    0    0   |  0  ||  0    0    0
 6    0    0   |  0  ||  0    0    0
 5    0    0   |  0  ||  0    0    0
 4    0    0   |  1  ||  0    0    1
 3    0    1   |  1  ||  1    0    0
 2    0    0   |  0  ||  0    0    0
 1    0    0   |  0  ||  0    0    0
 0    0    0   |  1  ||  0    0    1

quotient = 8, remainder = 1  (check: 25/3 = 8 r1)

dividend = 255  (0b11111111)

 i   r_in s_in | x_i || y_i  r_i  s_i
--   ---- ---- | --- || ---  ---  ---
 7    0    0   |  1  ||  0    0    1
 6    0    1   |  1  ||  1    0    0
 5    0    0   |  1  ||  0    0    1
 4    0    1   |  1  ||  1    0    0
 3    0    0   |  1  ||  0    0    1
 2    0    1   |  1  ||  1    0    0
 1    0    0   |  1  ||  0    0    1
 0    0    1   |  1  ||  1    0    0

quotient = 85, remainder = 0  (check: 255/3 = 85 r0)
*/