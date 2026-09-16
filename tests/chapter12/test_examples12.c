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

#include <stdio.h>

/*
  Floored modulo 2. C's % truncates toward zero, so -1 % 2 is -1,
  which is not a valid base -2 digit. This always returns 0 or 1.
*/
static int mod2_floored(int total) {
  return ((total % 2) + 2) % 2;
}

uint8_t add_base_negative_2(uint8_t a, uint8_t b, int *overflow) {
  uint8_t result = 0;
  int carry = 0;

  for (int i = 0; i < 8; i++) {
    int total = ((a >> i) & 1) + ((b >> i) & 1) + carry;
    int digit = mod2_floored(total);

    /*
      total = digit + 2*k, and 2*(-2)^i == -1 * (-2)^(i+1), therefore the
      k that this column sheds becomes a carry of -k into the next column
    */
    carry = -((total - digit) / 2);

    if (digit) {
      result |= (uint8_t)(1u << i);
    }
  }

  if (overflow != NULL) {
    *overflow = (carry != 0);
  }
  return result;
}

/*
  Renders a signed carry in the book's notation: -1 is written 11,
  since (-2)^(i+1) + (-2)^(i+2) == -(-2)^(i+1).
*/
static const char *carry_text(int carry) {
  if (carry == 0)  return "0";
  if (carry == 1)  return "1";
  if (carry == -1) return "11";
  return "??";
}

uint8_t add_base_negative_2_print_trace(uint8_t a, uint8_t b) {
  uint8_t result = 0;
  int carry = 0;

  printf("a = 0b");
  for (int i = 7; i >= 0; i--) { putchar(((a >> i) & 1) ? '1' : '0'); }
  printf("   b = 0b");
  for (int i = 7; i >= 0; i--) { putchar(((b >> i) & 1) ? '1' : '0'); }
  printf("\n\n");

  printf(" col  carry_in  a_i  b_i  total  digit  carry_out\n");
  printf(" ---  --------  ---  ---  -----  -----  ---------\n");

  for (int i = 0; i < 8; i++) {
    int carry_in = carry;
    int a_i = (a >> i) & 1;
    int b_i = (b >> i) & 1;
    int total = a_i + b_i + carry_in;
    int digit = mod2_floored(total);

    carry = -((total - digit) / 2);

    if (digit) { result |= (uint8_t)(1u << i); }

    printf(
      " %3d  %8s  %3d  %3d  %5d  %5d  %9s\n",
      i, carry_text(carry_in), a_i, b_i, total, digit, carry_text(carry)
    );
  }

  printf("\nresult = 0b");
  for (int i = 7; i >= 0; i--) { putchar(((result >> i) & 1) ? '1' : '0'); }
  printf("%s\n", (carry != 0) ? "   *** OVERFLOW ***" : "");
  return result;
}

/*
  Decodes without truncating to int8_t, so the expected value can be
  compared over the full [-170, 85] range that 8 base -2 digits
  actually span. from_base_negative_2 narrows its return to int8_t,
  which is the right thing for that function but would hide
  mismatches here.
*/
static int decode_full(uint8_t digits) {
  int result = 0;
  for (int i = 7; i >= 0; i--) {
    result = result * -2 + ((digits >> i) & 1);
  }
  return result;
}

Test(add_base_negative_2, book_example_addition) {
  int overflow = 1;
  uint8_t a = to_base_negative_2(19);
  uint8_t b = to_base_negative_2(-11);
  uint8_t sum = add_base_negative_2(a, b, &overflow);

  cr_assert_eq(sum, 0b00011000, "got 0x%02X want 0x%02X", sum, 0b00011000);
  cr_assert_eq(decode_full(sum), 8);
  cr_assert_eq(overflow, 0);
}

Test(add_base_negative_2, book_example_subtraction_as_addition) {
  int overflow = 1;
  uint8_t a = to_base_negative_2(21);
  uint8_t b = to_base_negative_2(38);
  uint8_t sum = add_base_negative_2(a, b, &overflow);

  cr_assert_eq(sum, 0b01001111, "got 0x%02X want 0x%02X", sum, 0b01001111);
  cr_assert_eq(decode_full(sum), 59);
  cr_assert_eq(overflow, 0);
}

Test(add_base_negative_2, identities) {
  int overflow;
  cr_assert_eq(add_base_negative_2(0, 0, &overflow), 0);
  cr_assert_eq(overflow, 0);

  /* 1 + 1 == 110 */
  cr_assert_eq(add_base_negative_2(0b1, 0b1, &overflow), 0b110);
  cr_assert_eq(overflow, 0);

  /* 11 + 1 == 0, i.e. (-1) + 1 == 0 */
  cr_assert_eq(add_base_negative_2(0b11, 0b1, &overflow), 0b0);
  cr_assert_eq(overflow, 0);
}

Test(add_base_negative_2, adding_zero_is_identity) {
  int overflow;
  for (int a = 0; a <= 255; a++) {
    uint8_t sum = add_base_negative_2((uint8_t)a, 0, &overflow);
    cr_assert_eq(sum, (uint8_t)a, "a=%d", a);
    cr_assert_eq(overflow, 0, "a=%d", a);
  }
}

Test(add_base_negative_2, commutative) {
  for (int a = 0; a <= 255; a++) {
    for (int b = 0; b <= 255; b++) {
      int ov_ab, ov_ba;
      uint8_t ab = add_base_negative_2((uint8_t)a, (uint8_t)b, &ov_ab);
      uint8_t ba = add_base_negative_2((uint8_t)b, (uint8_t)a, &ov_ba);
      cr_assert_eq(ab, ba, "a=%d b=%d", a, b);
      cr_assert_eq(ov_ab, ov_ba, "a=%d b=%d", a, b);
    }
  }
}

/*
  Exhaustive check over all 65536 digit-pattern pairs. Every pair
  where the sum fits in 8 base -2 digits must decode to the sum
  of the decoded operands; every pair that overflows must genuinely
  be out of range, which rules out the adder simply flagging
  overflow whenever it is unsure.
*/
Test(add_base_negative_2, exhaustive_against_decode) {
  for (int a = 0; a <= 255; a++) {
    for (int b = 0; b <= 255; b++) {
      int overflow;
      uint8_t sum = add_base_negative_2((uint8_t)a, (uint8_t)b, &overflow);
      int want = decode_full((uint8_t)a) + decode_full((uint8_t)b);

      if (overflow) {
        cr_assert(
          want < -170 || want > 85,
          "a=%d b=%d flagged overflow but %d is representable", a, b, want
        );
      } else {
        cr_assert_eq(
          decode_full(sum), want,
          "a=%d b=%d got=%d want=%d", a, b, decode_full(sum), want
        );
      }
    }
  }
}

Test(add_base_negative_2, overflow_past_max) {
  /*
    85 is the largest value 8 base -2 digits can hold
    therefore adding 1 must report overflow
  */
  int overflow = 0;
  add_base_negative_2(to_base_negative_2(85), to_base_negative_2(1), &overflow);
  cr_assert_eq(overflow, 1);
}

Test(add_base_negative_2, accepts_null_overflow) {
  cr_assert_eq(add_base_negative_2(0b1, 0b1, NULL), 0b110);
}

/*
  No assertions -- prints the book's two worked examples plus an
  overflow case.
*/
Test(add_base_negative_2, print_trace_demo) {
  printf("\n\n**  add_base_negative_2  **\n\n");
  add_base_negative_2_print_trace(
    to_base_negative_2(1),
    to_base_negative_2(1)
  );
  printf("\n");
  add_base_negative_2_print_trace(
    to_base_negative_2(2),
    to_base_negative_2(3)
  );
  printf("\n");
  add_base_negative_2_print_trace(
    to_base_negative_2(19),
    to_base_negative_2(-11)
  );
  printf("\n");
  add_base_negative_2_print_trace(
    to_base_negative_2(21),
    to_base_negative_2(38)
  );
}

/*
a = 0b00000001   b = 0b00000001

 col  carry_in  a_i  b_i  total  digit  carry_out
 ---  --------  ---  ---  -----  -----  ---------
   0         0    1    1      2      0         11
   1        11    0    0     -1      1          1
   2         1    0    0      1      1          0
   3         0    0    0      0      0          0
   4         0    0    0      0      0          0
   5         0    0    0      0      0          0
   6         0    0    0      0      0          0
   7         0    0    0      0      0          0

result = 0b00000110

a = 0b00000110   b = 0b00000111

 col  carry_in  a_i  b_i  total  digit  carry_out
 ---  --------  ---  ---  -----  -----  ---------
   0         0    0    1      1      1          0
   1         0    1    1      2      0         11
   2        11    1    1      1      1          0
   3         0    0    0      0      0          0
   4         0    0    0      0      0          0
   5         0    0    0      0      0          0
   6         0    0    0      0      0          0
   7         0    0    0      0      0          0

result = 0b00000101

a = 0b00010111   b = 0b00110101

 col  carry_in  a_i  b_i  total  digit  carry_out
 ---  --------  ---  ---  -----  -----  ---------
   0         0    1    1      2      0         11
   1        11    1    0      0      0          0
   2         0    1    1      2      0         11
   3        11    0    0     -1      1          1
   4         1    1    1      3      1         11
   5        11    0    1      0      0          0
   6         0    0    0      0      0          0
   7         0    0    0      0      0          0

result = 0b00011000

a = 0b00010101   b = 0b01111010

 col  carry_in  a_i  b_i  total  digit  carry_out
 ---  --------  ---  ---  -----  -----  ---------
   0         0    1    0      1      1          0
   1         0    0    1      1      1          0
   2         0    1    0      1      1          0
   3         0    0    1      1      1          0
   4         0    1    1      2      0         11
   5        11    0    1      0      0          0
   6         0    0    1      1      1          0
   7         0    0    0      0      0          0

result = 0b01001111
*/

int8_t *to_base_4(uint8_t n) {
  uint8_t base_negative_2 = to_base_negative_2(n);
  int8_t *result = malloc(4 * sizeof(int8_t));
  for (int i = 0; i < 4; i++) {
    result[i] = from_base_negative_2((base_negative_2 >> (2 * i)) & 0b11);
  }
  return result;
}

Test(to_base_4, known_values) {
  int8_t *result;

  /* to_base_negative_2(0) = 0b00000000 */
  result = to_base_4(0);
  cr_assert_eq(result[0], 0);
  cr_assert_eq(result[1], 0);
  cr_assert_eq(result[2], 0);
  cr_assert_eq(result[3], 0);
  free(result);

  /* to_base_negative_2(1) = 0b00000001 */
  result = to_base_4(1);
  cr_assert_eq(result[0], 1);
  cr_assert_eq(result[1], 0);
  cr_assert_eq(result[2], 0);
  cr_assert_eq(result[3], 0);
  free(result);

  /* to_base_negative_2(2) = 0b00000110 */
  result = to_base_4(2);
  cr_assert_eq(result[0], -2);
  cr_assert_eq(result[1], 1);
  cr_assert_eq(result[2], 0);
  cr_assert_eq(result[3], 0);
  free(result);
  
  /* to_base_negative_2(-14) = 0b11101110 */
  result = to_base_4(-14);
  cr_assert_eq(result[0], -2);
  cr_assert_eq(result[1], 1);
  cr_assert_eq(result[2], -1);
  cr_assert_eq(result[3], 0);
  free(result);
}