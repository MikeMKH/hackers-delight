#include <criterion/criterion.h>
#include <stdint.h>

uint8_t to_base_negative_2(int8_t n) {
  uint8_t result = 0;
  uint8_t bit = 1;

  while (n != 0) {
    int8_t q = n / -2;
    int8_t r = n % -2;
    if (r < 0) { r += 2; q += 1; }
    
    if (r) {
      result |= bit;
    }
    n = q;
    bit <<= 1;
  }
  return result;
}

int8_t from_base_negative_2(uint8_t digits) {
  int result = 0;
 
  for (int i = 7; i >= 0; i--) {
    int bit = (digits >> i) & 1;
    result = result * -2 + bit;
  }
  return (int8_t)result;
}

Test(to_base_negative_2, positive) {
  cr_assert_eq(to_base_negative_2(0),  0b0);
  cr_assert_eq(to_base_negative_2(1),  0b1);
  cr_assert_eq(to_base_negative_2(2),  0b110);
  cr_assert_eq(to_base_negative_2(3),  0b111);
  cr_assert_eq(to_base_negative_2(4),  0b100);
  cr_assert_eq(to_base_negative_2(5),  0b101);
  cr_assert_eq(to_base_negative_2(6),  0b11010);
  cr_assert_eq(to_base_negative_2(7),  0b11011);
  cr_assert_eq(to_base_negative_2(8),  0b11000);
  cr_assert_eq(to_base_negative_2(9),  0b11001);
  cr_assert_eq(to_base_negative_2(10), 0b11110);
  cr_assert_eq(to_base_negative_2(11), 0b11111);
  cr_assert_eq(to_base_negative_2(12), 0b11100);
  cr_assert_eq(to_base_negative_2(13), 0b11101);
  cr_assert_eq(to_base_negative_2(14), 0b10010);
  cr_assert_eq(to_base_negative_2(15), 0b10011);
}

Test(to_base_negative_2, negative) {
  cr_assert_eq(to_base_negative_2(-1),  0b11);
  cr_assert_eq(to_base_negative_2(-2),  0b10);
  cr_assert_eq(to_base_negative_2(-3),  0b1101);
  cr_assert_eq(to_base_negative_2(-4),  0b1100);
  cr_assert_eq(to_base_negative_2(-5),  0b1111);
  cr_assert_eq(to_base_negative_2(-6),  0b1110);
  cr_assert_eq(to_base_negative_2(-7),  0b1001);
  cr_assert_eq(to_base_negative_2(-8),  0b1000);
  cr_assert_eq(to_base_negative_2(-9),  0b1011);
  cr_assert_eq(to_base_negative_2(-10), 0b1010);
  cr_assert_eq(to_base_negative_2(-11), 0b110101);
  cr_assert_eq(to_base_negative_2(-12), 0b110100);
  cr_assert_eq(to_base_negative_2(-13), 0b110111);
  cr_assert_eq(to_base_negative_2(-14), 0b110110);
  cr_assert_eq(to_base_negative_2(-15), 0b110001);
}

Test(to_base_negative_2, round_trip) {
  /*
    8 negabinary digits can only represent positive values up to 85
    to represent all values from 86 to 127, we would need to return uint16_t
  */
  for (int16_t i = -128; i <= 85; i++) {
    uint8_t encoded = to_base_negative_2((int8_t)i);
    int8_t  decoded = from_base_negative_2(encoded);
    cr_assert_eq(decoded, (int8_t)i, "Failed for i = %d, got %d", i, decoded);
  }
}

Test(from_base_negative_2, known_values) {
  cr_assert_eq(from_base_negative_2(0b0), 0);
  cr_assert_eq(from_base_negative_2(0b1), 1);
  cr_assert_eq(from_base_negative_2(0b110), 2);
  cr_assert_eq(from_base_negative_2(0b111), 3);
  cr_assert_eq(from_base_negative_2(0b11010), 6);
  cr_assert_eq(from_base_negative_2(0b10011), 15);
}
 
/*
  decode's range extends past int8_t, 0b10101010 is the pattern
  with the most negative mathematical value (-170) since that
  doesn't fit int8_t, it truncates to 86 via
  implementation-defined narrowing
*/
Test(from_base_negative_2, out_of_encode_range_pattern) {
  cr_assert_eq(from_base_negative_2(0b10101010), 86);
}