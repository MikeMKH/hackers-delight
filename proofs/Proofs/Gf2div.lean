import Std.Tactic.BVDecide

/-
  Mirrors the bit-serial GF(2) division circuit discussed for
  chapter 14: an 8-bit dividend pushed MSB-first through a 3-bit
  shift register, feeding back the fixed divisor x^3+x+1 (0b1011)
  whenever a 1 bit shifts out the top. The divisor's own top bit is
  implicit (always 1), so the feedback value is just its low 3 bits,
  0b011.

  This is NOT a transcription of gf2_divide.c -- that function has a
  variable-length loop driven by __builtin_clz, which doesn't unfold
  into a fixed straight-line bv_decide goal. This models the other,
  hardware-shaped algorithm instead (the shift register CRC circuits
  actually run), independently verified in C to produce the same
  remainder and quotient as gf2_divide for this fixed divisor across
  all 256 possible 8-bit dividends before being written here.
-/

/-- One clock cycle of the shift register: shift in one dividend bit,
    feed back 0b011 if a 1 bit shifted out the top, and shift that
    same carry bit into the accumulating quotient. -/
def gf2Step (r : BitVec 3) (q : BitVec 8) (bit : BitVec 1) : BitVec 3 × BitVec 8 :=
  let carry := r >>> 2
  let r1 := ((r <<< 1) ||| bit.zeroExtend 3) &&& 0b111#3
  let r2 := if carry = 1#3 then r1 ^^^ 0b011#3 else r1
  let q' := (q <<< 1) ||| carry.zeroExtend 8
  (r2, q')

/-- Bit i of dividend, as a 1-bit value, via shift/mask/compare --
    the same primitives gf2Step itself uses, to keep everything in
    bv_decide's comfort zone. -/
def gf2Bit (dividend : BitVec 8) (i : Nat) : BitVec 1 :=
  if (dividend >>> i) &&& 1#8 = 1#8 then 1#1 else 0#1

/-- The register run for all 8 bits of the dividend, MSB first,
    unrolled by hand -- same style as parityXorTree's unrolled folds. -/
def gf2DivideBitSerial (dividend : BitVec 8) : BitVec 3 × BitVec 8 :=
  let (r1, q1) := gf2Step 0#3 0#8 (gf2Bit dividend 7)
  let (r2, q2) := gf2Step r1 q1 (gf2Bit dividend 6)
  let (r3, q3) := gf2Step r2 q2 (gf2Bit dividend 5)
  let (r4, q4) := gf2Step r3 q3 (gf2Bit dividend 4)
  let (r5, q5) := gf2Step r4 q4 (gf2Bit dividend 3)
  let (r6, q6) := gf2Step r5 q5 (gf2Bit dividend 2)
  let (r7, q7) := gf2Step r6 q6 (gf2Bit dividend 1)
  let (r8, q8) := gf2Step r7 q7 (gf2Bit dividend 0)
  (r8, q8)

/--
  Reconstruct the dividend from the register's own output: multiply
  quotient by x^3+x+1 via its own shift-XOR formula (bits at
  positions 3, 1, 0) rather than a general multiply routine, then XOR
  in the remainder. Widened to 11 bits (8 + the degree-3 shift) so
  the shift doesn't truncate real bits the way a same-width shift
  would.
-/
def gf2VerifyBitSerial (dividend : BitVec 8) : Prop :=
  let (remainder, quotient) := gf2DivideBitSerial dividend
  let q11 := quotient.zeroExtend 11
  let r11 := remainder.zeroExtend 11
  let reconstructed := (q11 <<< 3) ^^^ (q11 <<< 1) ^^^ q11 ^^^ r11
  reconstructed = dividend.zeroExtend 11

/--
  The shift register's quotient and remainder, multiplied back out
  by the fixed divisor's own formula, reconstruct every one of the
  256 possible 8-bit dividends.
-/
theorem gf2DivideBitSerial_correct : ∀ dividend : BitVec 8, gf2VerifyBitSerial dividend := by
  intro dividend
  simp only [gf2VerifyBitSerial, gf2DivideBitSerial, gf2Step, gf2Bit]
  bv_decide
