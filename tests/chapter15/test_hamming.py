import unittest
from functools import reduce
from itertools import combinations
from operator import xor

"""
Table 15-1, one row per information value 0..15 from Hacker's Delight.
Positions 1..7, left to right:  p0 p1 u3 p2 u2 u1 u0
"""
TABLE_15_1 = [
  "0000000",  #  0
  "1101001",  #  1
  "0101010",  #  2
  "1000011",  #  3
  "1001100",  #  4
  "0100101",  #  5
  "1100110",  #  6
  "0001111",  #  7
  "1110000",  #  8
  "0011001",  #  9
  "1011010",  # 10
  "0110011",  # 11
  "0111100",  # 12
  "1010101",  # 13
  "0010110",  # 14
  "1111111",  # 15
]


def xor_of(word, *positions):
  """
  positions are 1-based, left to right
  """
  return reduce(xor, (int(word[p - 1]) for p in positions))


def syndrome(word):
  """
  The receiver's three exclusive-ors. Read as a binary number the
  result is the position of a single-bit error (0 means no error).
  """
  return (4 * xor_of(word, 4, 5, 6, 7)
          + 2 * xor_of(word, 2, 3, 6, 7)
          + 1 * xor_of(word, 1, 3, 5, 7))


def flip(word, pos):
  return format(int(word, 2) ^ (1 << (7 - pos)), "07b")


def correct(word):
  s = syndrome(word)
  return word if s == 0 else flip(word, s)


def distance(a, b):
  return bin(int(a, 2) ^ int(b, 2)).count("1")


class TestHamming74(unittest.TestCase):

  def test_information_bits_sit_at_positions_3_5_6_7(self):
    for n, word in enumerate(TABLE_15_1):
      u3, u2, u1, u0 = word[2], word[4], word[5], word[6]
      self.assertEqual(int(u3 + u2 + u1 + u0, 2), n)

  def test_check_bits_follow_hamming_method(self):
    for n, word in enumerate(TABLE_15_1):
      u3, u2, u1, u0 = (n >> 3) & 1, (n >> 2) & 1, (n >> 1) & 1, n & 1
      p0 = u3 ^ u2 ^ u0  # covers positions 3, 5, 7
      p1 = u3 ^ u1 ^ u0  # covers positions 3, 6, 7
      p2 = u2 ^ u1 ^ u0  # covers positions 5, 6, 7
      self.assertEqual(word, f"{p0}{p1}{u3}{p2}{u2}{u1}{u0}")

  def test_every_codeword_has_zero_syndrome(self):
    for word in TABLE_15_1:
      self.assertEqual(syndrome(word), 0, word)

  def test_book_example_1001110(self):
    received = "1001110"  # row 4 with bit 6 flipped
    self.assertEqual(flip(TABLE_15_1[4], 6), received)
    self.assertEqual(xor_of(received, 1, 3, 5, 7), 0)
    self.assertEqual(xor_of(received, 2, 3, 6, 7), 1)
    self.assertEqual(xor_of(received, 4, 5, 6, 7), 1)
    self.assertEqual(syndrome(received), 0b110)  # 6: the bad bit is position 6
    self.assertEqual(correct(received), TABLE_15_1[4])

  def test_syndrome_is_the_error_position(self):
    for word in TABLE_15_1:
      for pos in range(1, 8):
        self.assertEqual(syndrome(flip(word, pos)), pos)
        self.assertEqual(correct(flip(word, pos)), word)

  def test_minimum_distance_is_3(self):
    self.assertEqual(min(distance(a, b) for a, b in combinations(TABLE_15_1, 2)), 3)

  def test_perfect_code(self):
    """
    16 codewords * (itself + 7 one-bit neighbors) = 128 = 2**7, so every
    7-bit word is within one flip of exactly one codeword.
    """
    for n in range(128):
      word = format(n, "07b")
      near = [c for c in TABLE_15_1 if distance(c, word) <= 1]
      self.assertEqual(len(near), 1, word)

  def test_two_bit_errors_are_silently_miscorrected(self):
    for word in TABLE_15_1:
      for a, b in combinations(range(1, 8), 2):
        received = flip(flip(word, a), b)
        self.assertNotEqual(syndrome(received), 0)  # looks like a one-bit error
        fixed = correct(received)
        self.assertIn(fixed, TABLE_15_1)            # a valid codeword...
        self.assertNotEqual(fixed, word)            # ...but the wrong one


if __name__ == "__main__":
  unittest.main()