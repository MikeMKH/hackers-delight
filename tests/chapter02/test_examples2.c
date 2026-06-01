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

/* ---- (x | y) >=u max(x, y) ---- */

Test(inequalities_among_logic_and_arithmetic, or_geq_max) {
    uint8_t x0 = 0b11110000;
    uint8_t y0 = 0b11001100;
    uint8_t or_val0  = x0 | y0;
    uint8_t max_val0 = x0 > y0 ? x0 : y0;
    cr_assert(or_val0 >= max_val0);

    uint8_t x1 = 0b10101010;
    uint8_t y1 = 0b01010101;
    uint8_t or_val1  = x1 | y1;
    uint8_t max_val1 = x1 > y1 ? x1 : y1;
    cr_assert(or_val1 >= max_val1);

    uint8_t x2 = 0b11111111;
    uint8_t y2 = 0b11111111;
    uint8_t or_val2  = x2 | y2;
    uint8_t max_val2 = x2 > y2 ? x2 : y2;
    cr_assert(or_val2 >= max_val2);
}

/* ---- (x & y) <=u min(x, y) ---- */

Test(inequalities_among_logic_and_arithmetic, and_leq_min) {
    uint8_t x0 = 0b11110000;
    uint8_t y0 = 0b11001100;
    uint8_t and_val0 = x0 & y0;
    uint8_t min_val0 = x0 < y0 ? x0 : y0;
    cr_assert(and_val0 <= min_val0);

    uint8_t x1 = 0b10101010;  /* and = 0, min = 0b01010101: loosest bound */
    uint8_t y1 = 0b01010101;
    uint8_t and_val1 = x1 & y1;
    uint8_t min_val1 = x1 < y1 ? x1 : y1;
    cr_assert(and_val1 <= min_val1);

    uint8_t x2 = 0b11111111;
    uint8_t y2 = 0b11111111;
    uint8_t and_val2 = x2 & y2;
    uint8_t min_val2 = x2 < y2 ? x2 : y2;
    cr_assert(and_val2 <= min_val2);
}

/* ---- |x - y| <=u (x ^ y) ---- */

static uint8_t udiff(uint8_t x, uint8_t y) {
    return (x > y) ? x - y : y - x;
}

Test(inequalities_among_logic_and_arithmetic, abs_diff_leq_xor) {
    uint8_t x0 = 0b11110000;
    uint8_t y0 = 0b11001100;
    cr_assert(udiff(x0, y0) <= (x0 ^ y0));

    uint8_t x1 = 0b10101010;
    uint8_t y1 = 0b01010101;
    cr_assert(udiff(x1, y1) <= (x1 ^ y1));

    uint8_t x2 = 0b11111111;
    uint8_t y2 = 0b11111111;
    cr_assert(udiff(x2, y2) <= (x2 ^ y2));

    uint8_t x3 = 0b00000000;
    uint8_t y3 = 0b00000000;
    cr_assert(udiff(x3, y3) <= (x3 ^ y3));
}

Test(absolute_value, abs_basic) {
    int8_t x = -42;
    int8_t e = 42;

    int8_t a = (x < 0) ? -x : x;
    cr_assert_eq(a, e);
}

Test(absolute_value, abs_xor_minus) {
    int8_t x = -42;
    int8_t e = 42;
    
    int8_t y = x >> 7;
    int8_t a = (x ^ y) - y;
    cr_assert_eq(a, e);
}

Test(absolute_value, abs_plus_xor) {
    int8_t x = -42;
    int8_t e = 42;
    
    int8_t y = x >> 7;
    int8_t a = (x + y) ^ y;
    cr_assert_eq(a, e);
}

Test(absolute_value, abs_minus_and) {
    int8_t x = -42;
    int8_t e = 42;
    
    int8_t y = x >> 7;
    /* int8_t a = x - (2*x & y); */
    int8_t a = x - (x<<1 & y); /* if you do not have multiplication */
    cr_assert_eq(a, e);
}

Test(absolute_value, nabs_basic) {
    int8_t x = 42;
    int8_t e = -42;

    int8_t a = (x < 0) ? x : -x;
    cr_assert_eq(a, e);
}

Test(absolute_value, nabs_xor_minus) {
    int8_t x = 42;
    int8_t e = -42;
    
    int8_t y = x >> 7;
    int8_t a = y - (x ^ y);
    cr_assert_eq(a, e);
}

Test(absolute_value, nabs_plus_xor) {
    int8_t x = 42;
    int8_t e = -42;
    
    int8_t y = x >> 7;
    int8_t a = (y - x) ^ y;
    cr_assert_eq(a, e);
}

Test(absolute_value, nabs_minus_and) {
    int8_t x = 42;
    int8_t e = -42;
    
    int8_t y = x >> 7;
    /* int8_t a = (2*x & y) - x; */
    int8_t a = (x<<1 & y) - x; /* if you do not have multiplication */
    cr_assert_eq(a, e);
}

Test(average, average_of_two_unsigned_ints_floor) {
    uint8_t x = 42;
    uint8_t y = 59;
    uint8_t e = 50;

    uint8_t a1 = (x + y) / 2;
    uint8_t a2 = (x & y) + ((x ^ y) >> 1);
    cr_assert_eq(a1, e);
    cr_assert_eq(a2, e);
}

Test(average, average_of_two_unsigned_ints_ceiling) {
    uint8_t x = 42;
    uint8_t y = 59;
    uint8_t e = 51;

    uint8_t a = (x | y) - ((x ^ y) >> 1);
    cr_assert_eq(a, e);
}

Test(average, average_of_two_signed_ints_both_negative) {
    int8_t x = -42;
    int8_t y = -9;
    int8_t e_floor     = -26;
    int8_t e_truncated = -25;

    uint8_t ux  = (uint8_t)x;
    uint8_t uy  = (uint8_t)y;
    uint8_t x_xor_y = ux ^ uy;

    uint8_t t  = (ux & uy) + (x_xor_y >> 1);
    int8_t  a1 = (int8_t)t;

    uint8_t correction = (t >> 7) & x_xor_y;
    int8_t  a2  = (int8_t)(t + correction);

    cr_assert_eq(a1, e_floor);
    cr_assert_eq(a2, e_truncated);
}

Test(sign_extension, sign_extend_8_to_32) {
    uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
    int32_t e = -42;

    int32_t a1 = (int8_t)x;
    int32_t a2 = ((x + 0x80) & 0xFF) - 0x80;
    int32_t a3 = ((x & 0xFF) ^ 0x80) - 0x80;
    int32_t a4 = (x & 0x7F) - (x & 0x80);
    cr_assert_eq(a1, e);
    cr_assert_eq(a2, e);
    cr_assert_eq(a3, e);
    cr_assert_eq(a4, e);
}