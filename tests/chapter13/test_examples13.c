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

/*
  Simulates the simplest possible single-track Gray code (STGC)
  rotational sensor -- the family the book calls "simple, and rather
  uninteresting," which is exactly what makes it a good starting
  point.
 
  Picture a ring with 2n positions around it. The first half is 0
  (nonconducting), the second half is 1 (conducting) -- one physical
  track, split straight down the middle. n brushes sit on that same
  ring, one angular position apart, instead of n separate rings at
  different radii. At any rotational position, the n brushes
  together read an n-bit code.
 
  Verified against the book's own n=2, n=3, n=4 tables: this
  construction reproduces them exactly, position for position. It
  generalizes to any n, though the book notes this family is not the
  best you can do -- see Figure 13-5's n=5 STGC (30 code words) for
  a genuinely optimized one, which this does not attempt to model.
*/
 
/*
  Reads the single physical track at a given position. position is
  taken mod 2n, so it wraps the way a physical ring does. Returns 0
  for the first half of the ring, 1 for the second half.
*/
uint8_t stgc_track_bit(uint8_t position, uint8_t n);
 
/*
  Reads all n brushes at once for a given rotational position,
  packed MSB first. Brush j samples the track at (position + j),
  for j = 0 .. n-1 -- n brushes, one step apart, all on the one
  track from stgc_track_bit.
*/
uint8_t stgc_read(uint8_t position, uint8_t n);
 
/*
  Prints the track and the code word read at every rotational
  position from 0 to 2n-1, as if slowly spinning the sensor one full
  turn and reading it at each step.
*/
void stgc_print_trace(uint8_t n);

#include <stdio.h>

uint8_t stgc_track_bit(uint8_t position, uint8_t n) {
  uint8_t track_len = (uint8_t)(2 * n);
  uint8_t p = (uint8_t)(position % track_len);
  return (uint8_t)(p < n ? 0 : 1);
}

uint8_t stgc_read(uint8_t position, uint8_t n) {
  uint8_t code = 0;

  for (uint8_t j = 0; j < n; j++) {
    uint8_t bit = stgc_track_bit((uint8_t)(position + j), n);
    code = (uint8_t)((code << 1) | bit);
  }
  return code;
}

void stgc_print_trace(uint8_t n) {
  uint8_t track_len = (uint8_t)(2 * n);

  printf("track (position 0..%u): ", (unsigned)(track_len - 1));
  for (uint8_t i = 0; i < track_len; i++) {
    putchar(stgc_track_bit(i, n) ? '1' : '0');
  }
  printf("\n\n");

  printf(" pos  code  (brush positions read)\n");
  printf(" ---  ----  -----------------------\n");

  for (uint8_t p = 0; p < track_len; p++) {
    uint8_t code = stgc_read(p, n);

    printf(" %3u  ", (unsigned)p);
    for (int b = n - 1; b >= 0; b--) {
      putchar(((code >> b) & 1) ? '1' : '0');
    }
    printf("  ");
    for (uint8_t j = 0; j < n; j++) {
      printf("%u", (unsigned)((p + j) % track_len));
      if (j + 1 < n) printf(",");
    }
    printf("\n");
  }
}

static int popcount8(uint8_t x) {
  int count = 0;
  while (x) {
    count += x & 1;
    x = (uint8_t)(x >> 1);
  }
  return count;
}

/*
  Exact digit-for-digit match against the book's own tables for
  n=2, n=3, n=4 -- Figure 13-1's worked examples. Confirms this is
  the same construction the text describes, not just a code that
  happens to have the right general properties.
*/
Test(stgc_read, matches_book_table_n2) {
  uint8_t want[] = {0b00, 0b01, 0b11, 0b10};
  for (uint8_t p = 0; p < 4; p++) {
    cr_assert_eq(stgc_read(p, 2), want[p], "p=%u", (unsigned)p);
  }
}

Test(stgc_read, matches_book_table_n3) {
  uint8_t want[] = {0b000, 0b001, 0b011, 0b111, 0b110, 0b100};
  for (uint8_t p = 0; p < 6; p++) {
    cr_assert_eq(stgc_read(p, 3), want[p], "p=%u", (unsigned)p);
  }
}

Test(stgc_read, matches_book_table_n4) {
  uint8_t want[] = {
    0b0000, 0b0001, 0b0011, 0b0111,
    0b1111, 0b1110, 0b1100, 0b1000,
  };
  for (uint8_t p = 0; p < 8; p++) {
    cr_assert_eq(stgc_read(p, 4), want[p], "p=%u", (unsigned)p);
  }
}

/*
  The defining property of a cyclic Gray code: every pair of
  positions that are physically adjacent on the ring -- including
  the wrap from the last position back to the first -- differ in
  exactly one bit. Checked generally, for several n, rather than
  just re-confirming the three hardcoded tables above.
*/
Test(stgc_read, cyclic_single_bit_change) {
  for (uint8_t n = 2; n <= 5; n++) {
    uint8_t track_len = (uint8_t)(2 * n);
    for (uint8_t p = 0; p < track_len; p++) {
      uint8_t here = stgc_read(p, n);
      uint8_t next = stgc_read((uint8_t)((p + 1) % track_len), n);
      cr_assert_eq(popcount8((uint8_t)(here ^ next)), 1,
                   "n=%u p=%u: %u -> %u changes %d bits, want 1",
                   (unsigned)n, (unsigned)p, here, next,
                   popcount8((uint8_t)(here ^ next)));
    }
  }
}

/*
  Single-track property, checked as an actual cross-check between
  the two functions rather than a value compared to itself: bit j of
  stgc_read's packed output (MSB first, so brush j lands at bit
  position n-1-j) must equal stgc_track_bit sampled at (p + j)
  directly. This is what makes it "single track" rather than an
  ordinary n-ring Gray code that happens to look similar -- and it
  would catch a bug where stgc_read's packing order drifted out of
  sync with stgc_track_bit's own indexing.
*/
Test(stgc_read, packs_bits_consistently_with_stgc_track_bit) {
  uint8_t n = 4;
  uint8_t track_len = (uint8_t)(2 * n);
  for (uint8_t p = 0; p < track_len; p++) {
    uint8_t code = stgc_read(p, n);
    for (uint8_t j = 0; j < n; j++) {
      uint8_t bit_from_read = (uint8_t)((code >> (n - 1 - j)) & 1);
      uint8_t bit_from_track = stgc_track_bit((uint8_t)(p + j), n);
      cr_assert_eq(bit_from_read, bit_from_track, "p=%u j=%u", (unsigned)p, (unsigned)j);
    }
  }
}

/*
  All 2n codes in one full rotation must be distinct -- a rotational
  sensor is useless if two different angles read the same code.
*/
Test(stgc_read, all_positions_distinct_within_one_rotation) {
  /*
    n capped at 5 deliberately: codes are n bits wide, and a
    32-bit `seen` bitmask can only shift by up to 31 without
    invoking undefined behavior. n=5 gives 5-bit codes (max 31),
    right at that boundary; n=6 would need a 64-bit mask instead.
  */
  for (uint8_t n = 2; n <= 5; n++) {
    uint8_t track_len = (uint8_t)(2 * n);
    uint32_t seen = 0;
    for (uint8_t p = 0; p < track_len; p++) {
      uint8_t code = stgc_read(p, n);
      cr_assert_eq((seen >> code) & 1, 0u,
                   "n=%u: code %u repeated at p=%u", (unsigned)n, code, (unsigned)p);
      seen |= (1u << code);
    }
  }
}

/*
  No assertions -- prints the track and the full rotation for n=4,
  matching Figure 13-1's table. Run with --verbose to see it.
*/
Test(stgc_print_trace, demo) {
  stgc_print_trace(4);
}