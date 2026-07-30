#include <criterion/criterion.h>
#include <stdlib.h>

/*
               truncating   modulus    floor
  7÷3           2  r1        2  r1      2  r1
  (-7)÷3       -2  r-1      -3  r2     -3  r2
  7÷(-3)       -2  r1       -2  r1     -3  r-2
  (-7)÷(-3)     2  r-1       3  r2      2  r-1
*/

/* truncating: C built-in behavior */
static int div_trunc(int u, int v) { return u / v; }
static int rem_trunc(int u, int v) { return u % v; }

/* modulus: remainder has same sign as divisor */
static int rem_mod(int u, int v) {
  int r = u % v;
  if (r != 0 && u < 0) r += (v < 0) ? -v : v;
  return r;
}
static int div_mod(int u, int v) {
  int q = u / v;
  int r = u % v;
  if (r != 0 && u < 0) q -= (v < 0) ? -1 : 1;
  return q;
}

/* floor: round toward negative infinity */
static int rem_floor(int u, int v) {
  int r = u % v;
  return r + (((r != 0) & ((u ^ v) < 0)) ? v : 0);
}
static int div_floor(int u, int v) {
  int r = u % v;
  int adjust = ((r != 0) & ((u ^ v) < 0)) ? 1 : 0;
  return u / v - adjust;
}

Test(division, truncating) {
  cr_assert_eq(div_trunc( 7,  3),  2); cr_assert_eq(rem_trunc( 7,  3),  1);
  cr_assert_eq(div_trunc(-7,  3), -2); cr_assert_eq(rem_trunc(-7,  3), -1);
  cr_assert_eq(div_trunc( 7, -3), -2); cr_assert_eq(rem_trunc( 7, -3),  1);
  cr_assert_eq(div_trunc(-7, -3),  2); cr_assert_eq(rem_trunc(-7, -3), -1);
}

Test(division, floor) {
  cr_assert_eq(div_floor( 7,  3),  2); cr_assert_eq(rem_floor( 7,  3),  1);
  cr_assert_eq(div_floor(-7,  3), -3); cr_assert_eq(rem_floor(-7,  3),  2);
  cr_assert_eq(div_floor( 7, -3), -3); cr_assert_eq(rem_floor( 7, -3), -2);
  cr_assert_eq(div_floor(-7, -3),  2); cr_assert_eq(rem_floor(-7, -3), -1);
}

Test(division, modulus) {
  cr_assert_eq(div_mod( 7,  3),  2); cr_assert_eq(rem_mod( 7,  3),  1);
  cr_assert_eq(div_mod(-7,  3), -3); cr_assert_eq(rem_mod(-7,  3),  2);
  cr_assert_eq(div_mod( 7, -3), -2); cr_assert_eq(rem_mod( 7, -3),  1);
  cr_assert_eq(div_mod(-7, -3),  3); cr_assert_eq(rem_mod(-7, -3),  2);
}

Test(division, invariant_q_times_v_plus_r_equals_u) {
  /* for all three: q*v + r == u must always hold */
  int us[] = {7, -7};
  int vs[] = {3, -3};
  for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) {
    int u = us[i], v = vs[j];
    cr_assert_eq(div_trunc(u,v)*v + rem_trunc(u,v), u, "trunc u=%d v=%d", u, v);
    cr_assert_eq(div_floor(u,v)*v + rem_floor(u,v), u, "floor u=%d v=%d", u, v);
    cr_assert_eq(div_mod(u,v)  *v + rem_mod(u,v),   u, "mod   u=%d v=%d", u, v);
  }
}