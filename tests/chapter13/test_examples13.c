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