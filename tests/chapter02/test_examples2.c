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
    int8_t a = x - (int8_t)((uint8_t)x << 1 & (uint8_t)y); /* if you do not have multiplication */
    /* left shifting a negative signed value is undefined behavior, so cast it as uint */
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

static void shift_right_signed_from_unsigned_test_helper(uint8_t x, int n, int8_t e) {
    int8_t a1 = (((int8_t)x) >> n);
    int8_t a2 = (int8_t)(((uint8_t)(x + 0x80u) >> n)) - (int8_t)(0x80u >> n);
    uint8_t t3 = 0x80u >> n;
    int8_t a3 = ((x >> n) ^ t3) - t3;
    uint8_t t4 = (x & 0x80u) >> n;
    int8_t a4 = (x >> n) - (t4 + t4);
    cr_assert_eq(a1, e);
    cr_assert_eq(a2, e);
    cr_assert_eq(a3, e);
    cr_assert_eq(a4, e);
}

Test(shift_right_signed_from_unsigned, n_equals_1) {
    uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
    int8_t e = -21;   /* -42 / 2 = -21, expected result */
    int n = 1;
    shift_right_signed_from_unsigned_test_helper(x, n, e);
}

Test(shift_right_signed_from_unsigned, n_equals_2) {
    uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
    int8_t e = -11;   /* -42 / 4 = -10.5, expected result */
    int n = 2;
    shift_right_signed_from_unsigned_test_helper(x, n, e);
}

Test(shift_right_signed_from_unsigned, n_equals_3) {
    uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
    int8_t e = -6;    /* -42 / 8 = -5.25, expected result */
    int n = 3;
    shift_right_signed_from_unsigned_test_helper(x, n, e);
}

Test(shift_right_signed_from_unsigned, n_equals_7) {
    uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
    int8_t e = -1;    /* -42 / 128 = -0.328125, expected result */
    int n = 7;
    shift_right_signed_from_unsigned_test_helper(x, n, e);
}

static int8_t sign_function(uint8_t x) {
  return ((int8_t)x >> 7) | (int8_t)((uint8_t)-x >> 7);
}

Test(sign_function, sign_function_given_negative_value) {
  uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
  int8_t e = -1;
  int8_t a = sign_function(x);
  cr_assert_eq(a, e);
}

Test(sign_function, sign_function_given_zero_value) {
  uint8_t x = 0x00; /* 0 as a raw unsigned byte */
  int8_t e = 0;
  int8_t a = sign_function(x);
  cr_assert_eq(a, e);
}

Test(sign_function, sign_function_given_positive_value) {
  uint8_t x = 0x2A; /* 42 as a raw unsigned byte */
  int8_t e = 1;
  int8_t a = sign_function(x);
  cr_assert_eq(a, e);
}

static int8_t compare_function(uint8_t x, uint8_t y) {
  return ((int8_t)(x - y) >> 7) | (int8_t)(((uint8_t)(y - x) >> 7));
}

Test(compare_function, compare_function_given_x_greater_than_y) {
  uint8_t x = 0x2A; /* 42 as a raw unsigned byte */
  uint8_t y = 0xD6; /* -42 as a raw unsigned byte */
  int8_t e = 1;
  int8_t a = compare_function(x, y);
  cr_assert_eq(a, e);
}

Test(compare_function, compare_function_given_x_less_than_y) {
  uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
  uint8_t y = 0x2A; /* 42 as a raw unsigned byte */
  int8_t e = -1;
  int8_t a = compare_function(x, y);
  cr_assert_eq(a, e);
}

Test(compare_function, compare_function_given_x_equals_y) {
  uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
  uint8_t y = 0xD6; /* -42 as a raw unsigned byte */
  int8_t e = 0;
  int8_t a = compare_function(x, y);
  cr_assert_eq(a, e);
}

Test(isign, transfer_negative_sign_from_y_to_x) {
  uint8_t x = 0x2A; /* 42 as a raw unsigned byte */
  uint8_t y = 0xD6; /* -42 as a raw unsigned byte */
  int8_t e = -42;
  
  int8_t t1 = (int8_t)y >> 7;
  int8_t a1 = (abs((int8_t)x) ^ t1) - t1;
  cr_assert_eq(a1, e);
  
  int8_t t2 = (int8_t)(x ^ y) >> 7;
  int8_t a2 = (x ^ t2) - t2;
  cr_assert_eq(a2, e);
}

Test(isign, transfer_positive_sign_from_y_to_x) {
  uint8_t x = 0xD6; /* -42 as a raw unsigned byte */
  uint8_t y = 0x2A; /* 42 as a raw unsigned byte */
  int8_t e = 42;
  
  int8_t t1 = (int8_t)y >> 7;
  int8_t a1 = (abs((int8_t)x) ^ t1) - t1;
  cr_assert_eq(a1, e);
  
  int8_t t2 = (int8_t)(x ^ y) >> 7;
  int8_t a2 = (x ^ t2) - t2;
  cr_assert_eq(a2, e);
}

static void check_decode_zero_means_8(uint8_t x, uint8_t e) {
  /* type sizes do not matter as long as they are 3-bit or larger */
  
  /* all formulas decode a 3-bit "zero means 8" field */
    uint8_t a1 = ((x - 1) & 7) + 1;
    uint8_t a2 = ((x + 7) & 7) + 1;
    uint8_t a3 = ((x - 1) | (uint8_t)-8) + 9;
    uint8_t a4 = ((x + 7) | (uint8_t)-8) + 9;
    uint8_t a5 = ((x + 7) | 8) - 7;
    uint8_t a6 = ((x - 1) & 8) + x;
    uint8_t a7 = 8 - ((uint8_t)-x & 7);
    uint8_t a8 = -((uint8_t)-x | (uint8_t)-8);

    cr_assert_eq(a1, e, "x=%u a1=%u", x, a1);
    cr_assert_eq(a2, e, "x=%u a2=%u", x, a2);
    cr_assert_eq(a3, e, "x=%u a3=%u", x, a3);
    cr_assert_eq(a4, e, "x=%u a4=%u", x, a4);
    cr_assert_eq(a5, e, "x=%u a5=%u", x, a5);
    cr_assert_eq(a6, e, "x=%u a6=%u", x, a6);
    cr_assert_eq(a7, e, "x=%u a7=%u", x, a7);
    cr_assert_eq(a8, e, "x=%u a8=%u", x, a8);
}

Test(zero_means_2_nth, x_equals_0_means_8) {
    check_decode_zero_means_8(0, 8);  /* 0 = 2^3 */
}

Test(zero_means_2_nth, x_equals_1) {
    check_decode_zero_means_8(1, 1);
}

Test(zero_means_2_nth, x_equals_4) {
    check_decode_zero_means_8(4, 4);
}

Test(zero_means_2_nth, x_equals_7) {
    check_decode_zero_means_8(7, 7);  /* maximum non-special value */
}

Test(add_subtract_multiply, subtraction_is_addition_of_negative) {
    int8_t x = 42;
    int8_t y = 58;
    int8_t e = x - y;

    int8_t a1 = x + (-y);
    int8_t a2 = x + ~y + 1;
    cr_assert_eq(a1, e);
    cr_assert_eq(a2, e);
}

Test(rotate_shifts, left_rotate) {
    uint8_t x  = 0b00010010; /* 18 as a raw unsigned byte */
    uint8_t e1 = 0b00100100; /* 36 as a raw unsigned byte */
    uint8_t e2 = 0b01001000; /* 72 as a raw unsigned byte */
    
    int8_t n1 = 1;
    uint8_t a1 = (x << n1) | (x >> (8 - n1));
    cr_assert_eq(a1, e1);
    
    int8_t n2 = 2;
    uint8_t a2 = (x << n2) | (x >> (8 - n2));
    cr_assert_eq(a2, e2);
}

Test(rotate_shifts, right_rotate) {
    uint8_t x  = 0b00010010; /* 18 as a raw unsigned byte */
    uint8_t e1 = 0b00001001; /* 9 as a raw unsigned byte */
    uint8_t e2 = 0b10000100; /* 132 as a raw unsigned byte */
    
    int8_t n1 = 1;
    uint8_t a1 = (x >> n1) | (x << (8 - n1));
    cr_assert_eq(a1, e1);
    
    int8_t n2 = 2;
    uint8_t a2 = (x >> n2) | (x << (8 - n2));
    cr_assert_eq(a2, e2);
}

Test(double_length_add, add_two_numbers) {
  uint8_t x1 = 0b01010011; /*  83 */
  uint8_t x0 = 0b10010010; /* 146 */
  uint8_t y1 = 0b10101001; /* 169 */
  uint8_t y0 = 0b11010110; /* 214 */
  /*
  x = (x1, x0) = 0b0101001110010010 =  83×256 + 146 = 21248 + 146 = 21394
  y = (y1, y0) = 0b1010100111010110 = 169×256 + 214 = 43264 + 214 = 43478
  e = x + y = 21394 + 43478 = 64872
   */
  uint16_t e = (uint16_t)(x1 << 8) + (uint16_t)(y1 << 8) + x0 + y0;

  uint8_t z0 = x0 + y0;
  uint8_t carry = ((x0 & y0) | ((x0 | y0) & (uint8_t)~z0)) >> 7;
  uint8_t z1 = x1 + y1 + carry;
  uint16_t a = (uint16_t)(z1 << 8) | z0;
  cr_assert_eq(a, e);
  cr_assert_eq(carry, 1);
  cr_assert_eq(z0, (146 + 214) & 0xFF);
  cr_assert_eq(z1, 83 + 169 + 1);
  cr_assert_eq(e, 64872);
}

Test(multibyte_add, add_four_8_bit_numbers) {
  uint8_t x3 = 0b01010011; /*  83 */
  uint8_t x2 = 0b10010010; /* 146 */
  uint8_t x1 = 0b10101001; /* 169 */
  uint8_t x0 = 0b11010110; /* 214 */
  uint32_t x = ((uint32_t)x3 << 24) | ((uint32_t)x2 << 16)
           | ((uint32_t)x1 <<  8) |  (uint32_t)x0;
  /*
  x = (x3, x2, x1, x0) = 0b01010011100100101010100111010110
                       = 83×256³ + 146×256² + 169×256¹ + 214×256⁰
                       = 1402668288
  */
 
  uint32_t y3 = 0b11110000; /* 240 */
  uint32_t y2 = 0b00001111; /* 15 */
  uint32_t y1 = 0b11110000; /* 240 */
  uint32_t y0 = 0b00001111; /* 15 */
  uint32_t y = ((uint32_t)y3 << 24) | ((uint32_t)y2 << 16)
             | ((uint32_t)y1 <<  8) |  (uint32_t)y0;
  /*
  y = (y3, y2, y1, y0) = 0b11110000000011111111000000001111
                       = 240×256³ + 15×256² + 240×256¹ + 15×256⁰
                       = 4039877788
  */
 
  /* expected: each byte sum truncated to 8 bits */
  uint8_t e3 = (uint8_t)(x3 + y3); /*  83 + 240 = 323 -> 67  */
  uint8_t e2 = (uint8_t)(x2 + y2); /* 146 +  15 = 161        */
  uint8_t e1 = (uint8_t)(x1 + y1); /* 169 + 240 = 409 -> 153 */
  uint8_t e0 = (uint8_t)(x0 + y0); /* 214 +  15 = 229        */
  
  uint32_t s = (x & 0x7F7F7F7F) + (y & 0x7F7F7F7F);
  s = (((x^y) & 0x80808080) ^ s);

  cr_assert_eq((uint8_t)( s        & 0xFF), e0, "byte 0");
  cr_assert_eq((uint8_t)((s >>  8) & 0xFF), e1, "byte 1");
  cr_assert_eq((uint8_t)((s >> 16) & 0xFF), e2, "byte 2");
  cr_assert_eq((uint8_t)((s >> 24) & 0xFF), e3, "byte 3");
}

Test(multibyte_subtraction, subtract_four_8_bit_numbers) {
  uint8_t x3 = 0b01010011; /*  83 */
  uint8_t x2 = 0b10010010; /* 146 */
  uint8_t x1 = 0b10101001; /* 169 */
  uint8_t x0 = 0b11010110; /* 214 */
  uint32_t x = ((uint32_t)x3 << 24) | ((uint32_t)x2 << 16)
           | ((uint32_t)x1 <<  8) |  (uint32_t)x0;
  /*
  x = (x3, x2, x1, x0) = 0b01010011100100101010100111010110
                       = 83×256³ + 146×256² + 169×256¹ + 214×256⁰
                       = 1402668288
  */
 
  uint32_t y3 = 0b11110000; /* 240 */
  uint32_t y2 = 0b00001111; /* 15 */
  uint32_t y1 = 0b11110000; /* 240 */
  uint32_t y0 = 0b00001111; /* 15 */
  uint32_t y = ((uint32_t)y3 << 24) | ((uint32_t)y2 << 16)
             | ((uint32_t)y1 <<  8) |  (uint32_t)y0;
  /*
  y = (y3, y2, y1, y0) = 0b11110000000011111111000000001111
                       = 240×256³ + 15×256² + 240×256¹ + 15×256⁰
                       = 4039877788
  */
 
  /* expected: each byte difference truncated to 8 bits */
  uint8_t e3 = (uint8_t)(x3 - y3); /*  83 - 240 = -157 -> 99  */
  uint8_t e2 = (uint8_t)(x2 - y2); /* 146 -  15 = 131        */
  uint8_t e1 = (uint8_t)(x1 - y1); /* 169 - 240 = -71 -> 185 */
  uint8_t e0 = (uint8_t)(x0 - y0); /* 214 -  15 = 199        */
  
  uint32_t d = (x | 0x80808080) - (y & 0x7F7F7F7F);
  uint32_t xnor_left = (x ^ y) | 0x7F7F7F7F;
  d = ~(xnor_left ^ d);

  cr_assert_eq((uint8_t)( d        & 0xFF), e0, "byte 0");
  cr_assert_eq((uint8_t)((d >>  8) & 0xFF), e1, "byte 1");
  cr_assert_eq((uint8_t)((d >> 16) & 0xFF), e2, "byte 2");
  cr_assert_eq((uint8_t)((d >> 24) & 0xFF), e3, "byte 3");
}

static int32_t doz(int32_t x, int32_t y) {
  return (x < y) ? 0 : (x - y);
}

Test(doz, doz_test_with_x_lt_y) {
  int32_t x = 12345678;
  int32_t y = 87654321;
  int32_t z = doz(x, y);
  cr_assert_eq(z, 0);
}

Test(doz, doz_test_with_x_gt_y) {
  int32_t x = 87654321;
  int32_t y = 12345678;
  int32_t z = doz(x, y);
  cr_assert_eq(z, 87654321 - 12345678);
}

static uint32_t dozu(uint32_t x, uint32_t y) {
  return (x < y) ? 0 : (x - y);
}

Test(dozu, dozu_test_with_x_lt_y) {
  uint32_t x = 0x12345678;
  uint32_t y = 0x87654321;
  uint32_t z = dozu(x, y);
  cr_assert_eq(z, 0);
}

Test(dozu, dozu_test_with_x_gt_y) {
  uint32_t x = 0x87654321;
  uint32_t y = 0x12345678;
  uint32_t z = dozu(x, y);
  cr_assert_eq(z, 0x87654321 - 0x12345678);
}

static int32_t doz_max(int32_t x, int32_t y) {
  return y + doz(x, y);
}

Test(doz_max, doz_max_test_with_x_lt_y) {
  int32_t x = 12345678;
  int32_t y = 87654321;
  int32_t z = doz_max(x, y);
  cr_assert_eq(z, 87654321);
}

Test(doz_max, doz_max_test_with_x_gt_y) {
  int32_t x = 87654321;
  int32_t y = 12345678;
  int32_t z = doz_max(x, y);
  cr_assert_eq(z, 87654321);
}

static int32_t doz_min(int32_t x, int32_t y) {
  return x - doz(x, y);
}

Test(doz_min, doz_min_test_with_x_lt_y) {
  int32_t x = 12345678;
  int32_t y = 87654321;
  int32_t z = doz_min(x, y);
  cr_assert_eq(z, 12345678);
}

Test(doz_min, doz_min_test_with_x_gt_y) {
  int32_t x = 87654321;
  int32_t y = 12345678;
  int32_t z = doz_min(x, y);
  cr_assert_eq(z, 12345678);
}

static uint32_t dozu_max(uint32_t x, uint32_t y) {
  return y + dozu(x, y);
}

Test(dozu_max, dozu_max_test_with_x_lt_y) {
  uint32_t x = 0x12345678;
  uint32_t y = 0x87654321;
  uint32_t z = dozu_max(x, y);
  cr_assert_eq(z, 0x87654321);
}

Test(dozu_max, dozu_max_test_with_x_gt_y) {
  uint32_t x = 0x87654321;
  uint32_t y = 0x12345678;
  uint32_t z = dozu_max(x, y);
  cr_assert_eq(z, 0x87654321);
}

static uint32_t dozu_min(uint32_t x, uint32_t y) {
  return x - dozu(x, y);
}

Test(dozu_min, dozu_min_test_with_x_lt_y) {
  uint32_t x = 0x12345678;
  uint32_t y = 0x87654321;
  uint32_t z = dozu_min(x, y);
  cr_assert_eq(z, 0x12345678);
}

Test(dozu_min, dozu_min_test_with_x_gt_y) {
  uint32_t x = 0x87654321;
  uint32_t y = 0x12345678;
  uint32_t z = dozu_min(x, y);
  cr_assert_eq(z, 0x12345678);
}