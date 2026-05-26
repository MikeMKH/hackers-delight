#include <criterion/criterion.h>
#include <stdlib.h>
#include <stdint.h>

Test(basics, turn_off_rightmost_1_bit) {
  int8_t x = 0b01011000;
  int8_t e = 0b01010000;
  int8_t a = x & (x - 1);
  cr_assert_eq(a, e);
}

Test(basics, turn_on_rightmost_0_bit) {
  int8_t x = 0b10100111;
  int8_t e = 0b10101111;
  int8_t a = x | (x + 1);
  cr_assert_eq(a, e);
}

Test(basics, turn_off_trailing_1_bits) {
  int8_t x = 0b10100111;
  int8_t e = 0b10100000;
  int8_t a = x & (x + 1);
  cr_assert_eq(a, e);
}

Test(basics, turn_on_trailing_0_bits) {
  int8_t x = 0b10101000;
  int8_t e = 0b10101111;
  int8_t a = x | (x - 1);
  cr_assert_eq(a, e);
}

Test(basics, word_with_1_bit_at_rightmost_0_bit) {
  int8_t x = 0b10100111;
  int8_t e = 0b00001000;
  int8_t a = ~x & (x + 1);
  cr_assert_eq(a, e);
}

Test(basics, word_with_0_bit_at_rightmost_1_bit) {
  int8_t x = 0b10101000;
  int8_t e = 0b11110111;
  int8_t a = ~x | (x - 1);
  cr_assert_eq(a, e);
}

Test(basics, word_with_1_bits_at_trailing_0_bits_rest_0_bits) {
  int8_t x = 0b01011000;
  int8_t e = 0b00000111;
  int8_t a = ~x & (x - 1);
  cr_assert_eq(a, e);
}

Test(basics, word_with_0_bits_at_trailing_1_bits_rest_1_bits) {
  int8_t x = 0b10100111;
  int8_t e = 0b11111000;
  int8_t a = ~x | (x + 1);
  cr_assert_eq(a, e);
}

Test(basics, isolate_rightmost_1_bit) {
  int8_t x = 0b01011000;
  int8_t e = 0b00001000;
  int8_t a = x & -x;
  cr_assert_eq(a, e);
}

Test(basics, word_with_1_bit_at_rightmost_1_bit_and_1_bits_at_trailing_0_bits_rest_0_bits) {
  int8_t x = 0b01011000;
  int8_t e = 0b00001111;
  int8_t a = x ^ (x - 1);
  cr_assert_eq(a, e);
}

Test(basics, word_with_1_bit_at_rightmost_0_bit_and_1_bits_at_trailing_1_bits_rest_0_bits) {
  int8_t x = 0b01010111;
  int8_t e = 0b00001111;
  int8_t a = x ^ (x + 1);
  cr_assert_eq(a, e);
}

Test(basics, turn_off_rightmost_contiguous_1_bits) {
  int8_t x = 0b01011100;
  int8_t e = 0b01000000;
  int8_t a = ((x & -x) + x) & x;
  cr_assert_eq(a, e);
}