#include <stddef.h>
#include <stdio.h>

#include "divide_by_3.h"

void div3_stage(div3_state_t in, uint8_t x_i, uint8_t *y_i, div3_state_t *out) {
  uint8_t r = (uint8_t)(in.r & 1);
  uint8_t s = (uint8_t)(in.s & 1);
  uint8_t x = (uint8_t)(x_i & 1);
  uint8_t nr = (uint8_t)(~r & 1);
  uint8_t ns = (uint8_t)(~s & 1);
  uint8_t nx = (uint8_t)(~x & 1);

  /* y_i = r_{i+1} + s_{i+1} x_i */
  *y_i = (uint8_t)((r | (uint8_t)(s & x)) & 1);

  /* r_i = !r_{i+1} s_{i+1} !x_i + r_{i+1} x_i */
  out->r = (uint8_t)((((uint8_t)(nr & s & nx)) | (uint8_t)(r & x)) & 1);

  /* s_i = !r_{i+1} !s_{i+1} x_i + r_{i+1} !s_{i+1} !x_i */
  out->s = (uint8_t)((((uint8_t)(nr & ns & x)) | (uint8_t)(r & ns & nx)) & 1);
}

uint8_t divide_by_3(uint8_t dividend, uint8_t *remainder) {
  return divide_by_3_traced(dividend, remainder, NULL);
}

uint8_t divide_by_3_traced(uint8_t dividend, uint8_t *remainder, div3_state_t *trace) {
  div3_state_t state = {0, 0};
  uint8_t quotient = 0;

  for (int i = 7; i >= 0; i--) {
    uint8_t x_i = (uint8_t)((dividend >> i) & 1);
    uint8_t y_i;
    div3_state_t next;

    div3_stage(state, x_i, &y_i, &next);
    quotient = (uint8_t)(quotient | (uint8_t)(y_i << i));
    state = next;

    if (trace != NULL) {
      trace[7 - i] = state;
    }
  }

  if (remainder != NULL) {
    *remainder = (uint8_t)(2 * state.r + state.s);
  }
  return quotient;
}

void divide_by_3_print_trace(uint8_t dividend) {
  div3_state_t state = {0, 0};
  uint8_t quotient = 0;

  printf("dividend = %3u  (0b", dividend);
  for (int b = 7; b >= 0; b--) {
    putchar(((dividend >> b) & 1) ? '1' : '0');
  }
  printf(")\n\n");

  printf(" i   r_in s_in | x_i || y_i  r_i  s_i\n");
  printf("--   ---- ---- | --- || ---  ---  ---\n");

  for (int i = 7; i >= 0; i--) {
    uint8_t x_i = (uint8_t)((dividend >> i) & 1);
    uint8_t y_i;
    div3_state_t next;

    div3_stage(state, x_i, &y_i, &next);
    quotient = (uint8_t)(quotient | (uint8_t)(y_i << i));

    printf(
      "%2d    %u    %u   |  %u  ||  %u    %u    %u\n",
      i, state.r, state.s, x_i,    y_i, next.r, next.s
    );

    state = next;
  }

  uint8_t remainder = (uint8_t)(2 * state.r + state.s);
  printf(
    "\nquotient = %u, remainder = %u  (check: %u/3 = %u r%u)\n",
    quotient, remainder, dividend, dividend / 3, dividend % 3
  );
}