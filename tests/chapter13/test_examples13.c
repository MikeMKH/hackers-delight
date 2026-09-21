#include <criterion/criterion.h>
#include <stdint.h>

uint8_t gray_code_to_binary(uint8_t binary) {
  return binary ^ (binary >> 1);
}

Test(gray_code_to_binary, test_gray_code_to_binary) {
  cr_assert_eq(gray_code_to_binary(0b00000000), 0b00000000);
  cr_assert_eq(gray_code_to_binary(0b00000001), 0b00000001);
  cr_assert_eq(gray_code_to_binary(0b00000010), 0b00000011);
  cr_assert_eq(gray_code_to_binary(0b00000011), 0b00000010);
  cr_assert_eq(gray_code_to_binary(0b00000100), 0b00000110);
  cr_assert_eq(gray_code_to_binary(0b00000101), 0b00000111);
  cr_assert_eq(gray_code_to_binary(0b00000110), 0b00000101);
  cr_assert_eq(gray_code_to_binary(0b00000111), 0b00000100);
  cr_assert_eq(gray_code_to_binary(0b00001000), 0b00001100);
  cr_assert_eq(gray_code_to_binary(0b00001001), 0b00001101);
  cr_assert_eq(gray_code_to_binary(0b00001010), 0b00001111);
  cr_assert_eq(gray_code_to_binary(0b00001011), 0b00001110);
  cr_assert_eq(gray_code_to_binary(0b00001100), 0b00001010);
  cr_assert_eq(gray_code_to_binary(0b00001101), 0b00001011);
  cr_assert_eq(gray_code_to_binary(0b00001110), 0b00001001);
  cr_assert_eq(gray_code_to_binary(0b00001111), 0b00001000);
}

uint8_t binary_to_gray_code(uint8_t gray) {
  uint8_t binary = gray;
  binary = binary ^ (binary >> 1);
  binary = binary ^ (binary >> 2);
  binary = binary ^ (binary >> 4);
  binary = binary ^ (binary >> 8);
  binary = binary ^ (binary >> 16);
  return binary;
}

Test(binary_to_gray_code, test_binary_to_gray_code) {
  cr_assert_eq(binary_to_gray_code(0b00000000), 0b00000000);
  cr_assert_eq(binary_to_gray_code(0b00000001), 0b00000001);
  cr_assert_eq(binary_to_gray_code(0b00000010), 0b00000011);
  cr_assert_eq(binary_to_gray_code(0b00000011), 0b00000010);
  cr_assert_eq(binary_to_gray_code(0b00000100), 0b00000111);
  cr_assert_eq(binary_to_gray_code(0b00000101), 0b00000110);
  cr_assert_eq(binary_to_gray_code(0b00000110), 0b00000100);
  cr_assert_eq(binary_to_gray_code(0b00000111), 0b00000101);
  cr_assert_eq(binary_to_gray_code(0b00001000), 0b00001111);
  cr_assert_eq(binary_to_gray_code(0b00001001), 0b00001110);
  cr_assert_eq(binary_to_gray_code(0b00001010), 0b00001100);
  cr_assert_eq(binary_to_gray_code(0b00001011), 0b00001101);
  cr_assert_eq(binary_to_gray_code(0b00001100), 0b00001000);
  cr_assert_eq(binary_to_gray_code(0b00001101), 0b00001001);
  cr_assert_eq(binary_to_gray_code(0b00001110), 0b00001011);
  cr_assert_eq(binary_to_gray_code(0b00001111), 0b00001010);
}

uint8_t increment_binary_nibble(uint8_t x) {
  uint8_t d = x & 1;
  uint8_t c = (x >> 1) & 1;
  uint8_t b = (x >> 2) & 1;
  uint8_t a = (x >> 3) & 1;

  uint8_t d_new = (uint8_t)(~d & 1);
  uint8_t c_new = (uint8_t)(c ^ d);
  uint8_t b_new = (uint8_t)(b ^ (c & d));
  uint8_t a_new = (uint8_t)(a ^ (b & c & d));

  return (uint8_t)((a_new << 3) | (b_new << 2) | (c_new << 1) | d_new);
}

Test(increment_binary_nibble, test_increment_binary_nibble) {
  cr_assert_eq(increment_binary_nibble(0b0000), 0b0001);
  cr_assert_eq(increment_binary_nibble(0b0001), 0b0010);
  cr_assert_eq(increment_binary_nibble(0b0010), 0b0011);
  cr_assert_eq(increment_binary_nibble(0b0011), 0b0100);
  cr_assert_eq(increment_binary_nibble(0b0100), 0b0101);
  cr_assert_eq(increment_binary_nibble(0b0101), 0b0110);
  cr_assert_eq(increment_binary_nibble(0b0110), 0b0111);
  cr_assert_eq(increment_binary_nibble(0b0111), 0b1000);
  cr_assert_eq(increment_binary_nibble(0b1000), 0b1001);
  cr_assert_eq(increment_binary_nibble(0b1001), 0b1010);
  cr_assert_eq(increment_binary_nibble(0b1010), 0b1011);
  cr_assert_eq(increment_binary_nibble(0b1011), 0b1100);
  cr_assert_eq(increment_binary_nibble(0b1100), 0b1101);
  cr_assert_eq(increment_binary_nibble(0b1101), 0b1110);
  cr_assert_eq(increment_binary_nibble(0b1110), 0b1111);
  cr_assert_eq(increment_binary_nibble(0b1111), 0b0000); // Wrap around
}

uint8_t increment_gray_code_nibble(uint8_t x) {
  uint8_t h = x & 1;
  uint8_t g = (x >> 1) & 1;
  uint8_t f = (x >> 2) & 1;
  uint8_t e = (x >> 3) & 1;

  uint8_t p = (uint8_t)(e ^ f ^ g ^ h);
  uint8_t h_new = (uint8_t)(h ^ (1 ^ p));
  uint8_t g_new = (uint8_t)(g ^ (h & p));
  uint8_t f_new = (uint8_t)(f ^ (g & (1 ^ h) & p));
  uint8_t e_new = (uint8_t)(e ^ ((f & (1 ^ g) & (1 ^ h) & p) |
                                  (e & (1 ^ f) & (1 ^ g) & (1 ^ h) & p)));

  return (uint8_t)((e_new << 3) | (f_new << 2) | (g_new << 1) | h_new);
}

Test(increment_gray_code_nibble, test_increment_gray_code_nibble) {
  cr_assert_eq(increment_gray_code_nibble(0b0000), 0b0001);
  cr_assert_eq(increment_gray_code_nibble(0b0001), 0b0011);
  cr_assert_eq(increment_gray_code_nibble(0b0011), 0b0010);
  cr_assert_eq(increment_gray_code_nibble(0b0010), 0b0110);
  cr_assert_eq(increment_gray_code_nibble(0b0110), 0b0111);
  cr_assert_eq(increment_gray_code_nibble(0b0111), 0b0101);
  cr_assert_eq(increment_gray_code_nibble(0b0101), 0b0100);
  cr_assert_eq(increment_gray_code_nibble(0b0100), 0b1100);
  cr_assert_eq(increment_gray_code_nibble(0b1100), 0b1101);
  cr_assert_eq(increment_gray_code_nibble(0b1101), 0b1111);
  cr_assert_eq(increment_gray_code_nibble(0b1111), 0b1110);
  cr_assert_eq(increment_gray_code_nibble(0b1110), 0b1010);
  cr_assert_eq(increment_gray_code_nibble(0b1010), 0b1011);
  cr_assert_eq(increment_gray_code_nibble(0b1011), 0b1001);
  cr_assert_eq(increment_gray_code_nibble(0b1001), 0b1000);
  cr_assert_eq(increment_gray_code_nibble(0b1000), 0b0000); // Wrap around
}