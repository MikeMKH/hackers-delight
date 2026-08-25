import unittest


def magicgu(nmax, d):
    """
    Figure 10-4 from Hacker's Delight.
    nc is the largest multiple of d that is <= nmax.
    """
    nc = (nmax // d) * d
    nbits = len(bin(nmax)) - 2
    for p in range(0, 2 * nbits + 1):
        if 2**p > nc * (d - 1 - (2**p - 1) % d):
            m = (2**p + d - 1 - (2**p - 1) % d) // d
            return (m, p, nc)
    raise RuntimeError(f"magicgu({nmax}, {d}): can't find p")


def apply_magic(n, m, p):
    return (m * n) >> p


class TestMagicgu(unittest.TestCase):

    def test_book_example_nmax90_d7(self):
        m, p, nc = magicgu(90, 7)
        self.assertEqual(m, 37, f"got m={m}")
        self.assertEqual(p, 8,  f"got p={p}")

    def test_book_example_nmax127_d7(self):
        m, p, nc = magicgu(127, 7)
        self.assertEqual(m, 147, f"got m={m}")
        self.assertEqual(p, 10,  f"got p={p}")

    def test_apply_magic_nmax90_d7(self):
        """Magic (37,8) is valid for n in [0, nc) not necessarily [0, 90]"""
        m, p, nc = magicgu(90, 7)
        self.assertEqual(nc, 84, f"nc should be 84 (largest multiple of 7 <= 90)")
        for n in range(0, nc):   # test up to nc, not nmax
            self.assertEqual(apply_magic(n, m, p), n // 7, f"n={n}")

    def test_apply_magic_nmax127_d7(self):
        m, p, nc = magicgu(127, 7)
        for n in range(0, nc):
            self.assertEqual(apply_magic(n, m, p), n // 7, f"n={n}")

    def test_magic_number_size_benefit(self):
        m90,  p90,  _ = magicgu(90,  7)
        m127, p127, _ = magicgu(127, 7)
        m255, p255, _ = magicgu(255, 7)
        self.assertLess(m90,  m127, "tighter nmax should give smaller m")
        self.assertLess(m127, m255, "tighter nmax should give smaller m")
        self.assertLess(p90,  p127, "tighter nmax should give smaller p")

    def test_various_divisors_full_byte(self):
        for d in range(2, 21):
            m, p, nc = magicgu(255, d)
            for n in range(0, nc):
                self.assertEqual(apply_magic(n, m, p), n // d,
                    f"d={d} n={n}")

    def test_power_of_2_divisors(self):
        for k in range(1, 8):
            d = 2**k
            m, p, nc = magicgu(255, d)
            for n in range(0, nc):
                self.assertEqual(apply_magic(n, m, p), n // d, f"d={d} n={n}")

    def test_large_nmax(self):
        m, p, nc = magicgu(0xFFFFFFFF, 7)
        for n in [0, 1, 6, 7, 8, 0x7FFFFFFF, 0xFFFFFFFE]:
            self.assertEqual(apply_magic(n, m, p), n // 7, f"n={n:#010x}")

    def test_correctness_range(self):
        for d in range(2, 50):
            for nmax in [63, 90, 127, 255]:
                m, p, nc = magicgu(nmax, d)
                for n in range(0, nc):   # valid range is [0, nc)
                    self.assertEqual(apply_magic(n, m, p), n // d,
                        f"d={d} nmax={nmax} nc={nc} n={n}")

    def test_nc_is_always_multiple_of_d(self):
        """nc should always be the largest multiple of d <= nmax"""
        for d in range(2, 20):
            for nmax in [63, 90, 127, 255]:
                m, p, nc = magicgu(nmax, d)
                self.assertEqual(nc % d, 0,
                    f"nc={nc} should be multiple of d={d}")
                self.assertLessEqual(nc, nmax,
                    f"nc={nc} should be <= nmax={nmax}")
                self.assertGreater(nc + d, nmax,
                    f"nc={nc} should be the LARGEST multiple of d={d} <= nmax={nmax}")


if __name__ == "__main__":
    unittest.main(verbosity=2)