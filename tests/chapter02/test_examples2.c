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

Test(de_morgan_laws_extended, not_over_and) {
  int8_t x = 0b10101010;
  int8_t y = 0b11001100;
  int8_t a = ~(x & y);
  int8_t b = ~x | ~y;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_or) {
  int8_t x = 0b10101010;
  int8_t y = 0b11001100;
  int8_t a = ~(x | y);
  int8_t b = ~x & ~y;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_plus_1) {
  int8_t x = 0b10101010;
  int8_t a = ~(x + 1);
  int8_t b = ~x - 1;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_minus_1) {
  int8_t x = 0b10101010;
  int8_t a = ~(x - 1);
  int8_t b = ~x + 1;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_negation) {
  int8_t x = 0b10101010;
  int8_t a = ~(-x);
  int8_t b = x - 1;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_xor) {
  int8_t x = 0b10101010;
  int8_t y = 0b11001100;
  int8_t a = ~(x ^ y);
  int8_t b = ~x ^ y;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_bitwise_equivalence) {
  int8_t x = 0b10101010;
  int8_t y = 0b11001100;
  int8_t a = ~(~(x ^ y));
  int8_t b = x ^ y;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_addition) {
  int8_t x = 0b10101010;
  int8_t y = 0b11001100;
  int8_t a = ~(x + y);
  int8_t b = ~x - y;
  cr_assert_eq(a, b);
}

Test(de_morgan_laws_extended, not_over_subtraction) {
  int8_t x = 0b10101010;
  int8_t y = 0b11001100;
  int8_t a = ~(x - y);
  int8_t b = ~x + y;
  cr_assert_eq(a, b);
}

Test(right_to_left_computability_test, snoob) {
  u_int16_t x = 0b010011110000;
  u_int16_t e = 0b010100000111;
  
  u_int16_t smallest, ripple, ones;
  smallest = x & -x;
  ripple = x + smallest;
  ones = x ^ ripple;
  ones = (ones >> 2) / smallest;
  u_int16_t a = ripple | ones;
  cr_assert_eq(a, e);
}

Test(addition_combined_with_logical_operators, negative) {
  int8_t x = 42;
  int8_t e = -x;
  int8_t a1 = ~x + 1;
  int8_t a2 = ~(x - 1);
  cr_assert_eq(a1, e);
  cr_assert_eq(a2, e);
}

Test(addition_combined_with_logical_operators, not) {
  int8_t x = 42;
  int8_t e = ~x;
  int8_t a = -x - 1;
  cr_assert_eq(a, e);
}

Test(addition_combined_with_logical_operators, negative_not) {
  int8_t x = 42;
  int8_t e = -~x;
  int8_t a = x + 1;
  cr_assert_eq(a, e);
}

Test(addition_combined_with_logical_operators, not_negative) {
  int8_t x = 42;
  int8_t e = ~-x;
  int8_t a = x - 1;
  cr_assert_eq(a, e);
}

Test(addition_combined_with_logical_operators, addition) {
  int8_t x = 42;
  int8_t y = 58;
  int8_t e = x + y;
  int8_t a1 = x - ~y - 1;
  int8_t a2 = (x ^ y) + 2 * (x & y);
  int8_t a3 = (x | y) + (x & y);
  int8_t a4 = 2 * (x | y) - (x ^ y);
  cr_assert_eq(a1, e);
  cr_assert_eq(a2, e);
  cr_assert_eq(a3, e);
  cr_assert_eq(a4, e);
}

Test(addition_combined_with_logical_operators, subtraction) {
  int8_t x = 42;
  int8_t y = 58;
  int8_t e = x - y;
  int8_t a1 = x + ~y + 1;
  int8_t a2 = (x ^ y) - 2 * (~x & y);
  int8_t a3 = (x & ~y) - (~x & y);
  int8_t a4 = 2 * (x & ~y) - (x ^ y);
  cr_assert_eq(a1, e);
  cr_assert_eq(a2, e);
  cr_assert_eq(a3, e);
  cr_assert_eq(a4, e);
}

Test(addition_combined_with_logical_operators, xor) {
  int8_t x = 42;
  int8_t y = 58;
  int8_t e = x ^ y;
  int8_t a = (x | y) - (x & y);
  cr_assert_eq(a, e);
}

Test(addition_combined_with_logical_operators, x_and_not_y) {
  int8_t x = 42;
  int8_t y = 58;
  int8_t e = x & ~y;
  int8_t a1 = (x | y) - y;
  int8_t a2 = x - (x & y);
  cr_assert_eq(a1, e);
  cr_assert_eq(a2, e);
}

Test(addition_combined_with_logical_operators, equivalence) {
  int8_t x = 0b01101010;  //  106
  int8_t y = 0b00111100;  //   60
  int8_t e = ~(x ^ y);
  int8_t a1 = (x & y) - (x | y) - 1;
  int8_t a2 = (x & y) + ~(x | y);
  cr_assert_eq(a1, e);
  cr_assert_eq(a2, e);
}

Test(addition_combined_with_logical_operators, or) {
  int8_t x = 42;
  int8_t y = 58;
  int8_t e = x | y;
  int8_t a = (x & ~y) + y;
  cr_assert_eq(a, e);
}

Test(addition_combined_with_logical_operators, and) {
  int8_t x = 42;
  int8_t y = 58;
  int8_t e = x & y;
  int8_t a = (~x | y) - ~x;
  cr_assert_eq(a, e);
}