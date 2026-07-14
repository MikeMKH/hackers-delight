#include <criterion/criterion.h>
#include <stdlib.h>

int zbytel(uint32_t x) {
  if      ((x >> 24)        == 0) return 0;
  else if ((x & 0x00FF0000) == 0) return 1;
  else if ((x & 0x0000FF00) == 0) return 2;
  else if ((x & 0x000000FF) == 0) return 3;
  else return 4;
}

Test(zbytel, examples) {
  cr_assert_eq(zbytel(0x00000000), 0);
  cr_assert_eq(zbytel(0x00333333), 0);
  cr_assert_eq(zbytel(0xFF000000), 1);
  cr_assert_eq(zbytel(0xFF003333), 1);
  cr_assert_eq(zbytel(0xFFFF0000), 2);
  cr_assert_eq(zbytel(0xFFFF0033), 2);
  cr_assert_eq(zbytel(0xFFFFFF00), 3);
  cr_assert_eq(zbytel(0xFFFFFFFF), 4);
}

int zbyter(uint32_t x) {
  if      ((x & 0x000000FF) == 0) return 0;
  else if ((x & 0x0000FF00) == 0) return 1;
  else if ((x & 0x00FF0000) == 0) return 2;
  else if ((x >> 24)        == 0) return 3;
  else return 4;
}

Test(zbyter, examples) {
  cr_assert_eq(zbyter(0x00000000), 0);
  cr_assert_eq(zbyter(0x33333300), 0);
  cr_assert_eq(zbyter(0x000000FF), 1);
  cr_assert_eq(zbyter(0x333300FF), 1);
  cr_assert_eq(zbyter(0x0000FFFF), 2);
  cr_assert_eq(zbyter(0x3300FFFF), 2);
  cr_assert_eq(zbyter(0x00FFFFFF), 3);
  cr_assert_eq(zbyter(0xFFFFFFFF), 4);
}

#include <stddef.h>
#include <string.h>
#include <stdint.h>

/*
  Casting char * to uint32_t * and dereferencing is undefined behavior
  if the pointer isn't 4-byte aligned.
*/
/*
size_t strlen_zbyter(const char *s) {
  const uint32_t *p = (const uint32_t *)s;
  size_t n = 0;
  while (1) {
    uint32_t w = *p++;
    int z = zbyter(w);
    if (z < 4) return n + z;
    n += 4;
  }
}
*/
/* returns nonzero if any byte in w is zero */
static uint32_t has_zero_byte(uint32_t w) {
    return (w - 0x01010101u) & ~w & 0x80808080u;
}

size_t strlen_zbyter(const char *s) {
    const char *p = s;

    /* byte-at-a-time until 4-byte aligned */
    while ((uintptr_t)p & 3) {
        if (*p == '\0') return (size_t)(p - s);
        p++;
    }

    /* word-at-a-time: only call zbyter once we know a zero byte exists */
    const uint32_t *w = (const uint32_t *)p;
    while (!has_zero_byte(*w)) {
        w++;
    }
    return (size_t)((const char *)w - s) + zbyter(*w);
}

/* allocate a string with enough padding for word-at-a-time reads */
static char *make_str(const char *s) {
  size_t len = strlen(s);
  /* allocate extra 4 bytes so word reads past the null are safe */
  char *p = calloc(len + 4, 1);
  memcpy(p, s, len);
  return p;
}

Test(strlen_zbyter, examples) {
  char *s;
  s = make_str(""); cr_assert_eq(strlen_zbyter(s), 0); free(s);
  s = make_str("A"); cr_assert_eq(strlen_zbyter(s), 1); free(s);
  s = make_str("abc"); cr_assert_eq(strlen_zbyter(s), 3); free(s);
  s = make_str("abcd"); cr_assert_eq(strlen_zbyter(s), 4); free(s);
  s = make_str("abcdefg"); cr_assert_eq(strlen_zbyter(s), 7); free(s);
  s = make_str("Mike Harris"); cr_assert_eq(strlen_zbyter(s), 11); free(s);
}

Test(strlen_zbyter, agrees_with_libc) {
  const char *cases[] = {
    "", "a", "ab", "abc", "abcd", "abcde",
    "hello world", "the quick brown fox", NULL
  };
  for (int i = 0; cases[i] != NULL; i++) {
    char *s = make_str(cases[i]);
    cr_assert_eq(
      strlen_zbyter(s), strlen(cases[i]),
      "mismatch for \"%s\"", cases[i]
    );
    free(s);
  }
}

int ffstr1_loop(uint32_t x, int n) {
  int k, p;
  p = 0;
  while (x != 0) {
    k = __builtin_clz(x);
    x <<= k;
    p += k;
    /* count leading 1's and guard clz(0) undefined behavior when x is all-ones */
    k = (x == 0xFFFFFFFFu) ? 32 : __builtin_clz(~x);
    if (k >= n) return p;
    x <<= k;
    p += k;
  }
  return 32;
}

Test(ffstr1_loop, examples) {
  cr_assert_eq(ffstr1_loop(0x00000000, 1),  32);
  cr_assert_eq(ffstr1_loop(0x00000000, 32), 32);
  cr_assert_eq(ffstr1_loop(0xF0000000, 1),  0);
  cr_assert_eq(ffstr1_loop(0xF0000000, 4),  0);
  cr_assert_eq(ffstr1_loop(0xF0000000, 5),  32);
  cr_assert_eq(ffstr1_loop(0xF000FF00, 5),  16);
  cr_assert_eq(ffstr1_loop(0xF000FFFF, 5),  16);
  cr_assert_eq(ffstr1_loop(0xFFFFFFFF, 32), 0);
  cr_assert_eq(ffstr1_loop(0x0000000F, 4),  28);
  cr_assert_eq(ffstr1_loop(0x00F00000, 4),  8);
  cr_assert_eq(ffstr1_loop(0x55555555, 2),  32); /* alternating, no run of 2 */
  cr_assert_eq(ffstr1_loop(0x00FF0000, 8),  8);
}

int ffstr1_shift_and_sequence(uint32_t x, int n) {
  int s;
  
  while (n > 1) {
    s = n >> 1;
    x = x & (x << s);
    n -= s;
  }
  /* count leading 1's and guard clz(0) undefined behavior when x is all-ones */
  return (x == 0) ? 32 : __builtin_clz(x);
}

Test(ffstr1_shift_and_sequence, examples) {
  cr_assert_eq(ffstr1_shift_and_sequence(0x00000000, 1),  32);
  cr_assert_eq(ffstr1_shift_and_sequence(0x00000000, 32), 32);
  cr_assert_eq(ffstr1_shift_and_sequence(0xF0000000, 1),  0);
  cr_assert_eq(ffstr1_shift_and_sequence(0xF0000000, 4),  0);
  cr_assert_eq(ffstr1_shift_and_sequence(0xF0000000, 5),  32);
  cr_assert_eq(ffstr1_shift_and_sequence(0xF000FF00, 5), 16);
  cr_assert_eq(ffstr1_shift_and_sequence(0xF000FFFF, 5), 16);
  cr_assert_eq(ffstr1_shift_and_sequence(0xFFFFFFFF, 32), 0);
  cr_assert_eq(ffstr1_shift_and_sequence(0x0000000F, 4), 28);
  cr_assert_eq(ffstr1_shift_and_sequence(0x00F00000, 4), 8);
  cr_assert_eq(ffstr1_shift_and_sequence(0x55555555, 2),  32); /* alternating, no run of 2 */
  cr_assert_eq(ffstr1_shift_and_sequence(0x00FF0000, 8),  8);
}

int maxstr1(uint32_t x) {
  int k;
  for (k = 0; x != 0; k++) x &= 2*x;
  return k;
}

Test(maxstr1, examples) {
  cr_assert_eq(maxstr1(0x00000000), 0);
  cr_assert_eq(maxstr1(0x00000001), 1);
  cr_assert_eq(maxstr1(0x22222203), 2);
  cr_assert_eq(maxstr1(0x00000007), 3);
  cr_assert_eq(maxstr1(0x0D0E0A0F), 4);
  cr_assert_eq(maxstr1(0x0F0F10FF), 8);
  cr_assert_eq(maxstr1(0x11110FFF), 12);
  cr_assert_eq(maxstr1(0xFFFFFFFF), 32);
}