import Std.Tactic.BVDecide

/-
  A Lean model of parity_xor_tree from src/chapter14/parity.c.

  This is NOT the C source imported into Lean -- it's a parallel
  translation using Lean's built-in BitVec type, kept in sync with
  parity.c by hand. The proof below is airtight about the algorithm
  as written here; it says nothing about whether parity.c actually
  matches it (that's still on the Criterion exhaustive test and on
  you reading both side by side).
-/

/-- Mirrors parity_xor_tree's three-step XOR-fold exactly. -/
def parityXorTree (b : BitVec 8) : BitVec 8 :=
  let x1 := b ^^^ (b >>> 4)
  let x2 := x1 ^^^ (x1 >>> 2)
  let x3 := x2 ^^^ (x2 >>> 1)
  x3 &&& 1

/--
  Naive reference spec: XOR together all 8 individual bits directly.
  Deliberately flat -- no recursion, no Nat.fold -- just the same
  BitVec shift/and/xor primitives parityXorTree itself uses, so
  bv_decide's bit-blaster can see straight through both sides once
  they're unfolded.
-/
def paritySpec (b : BitVec 8) : BitVec 8 :=
  (b &&& 1) ^^^ ((b >>> 1) &&& 1) ^^^ ((b >>> 2) &&& 1) ^^^ ((b >>> 3) &&& 1) ^^^
  ((b >>> 4) &&& 1) ^^^ ((b >>> 5) &&& 1) ^^^ ((b >>> 6) &&& 1) ^^^ ((b >>> 7) &&& 1)

/--
  The XOR-tree agrees with the naive bit-by-bit spec for all 256
  possible bytes. `simp only` unfolds both definitions down to plain
  BitVec operations first -- without it, bv_decide treats each
  function call as an opaque variable and reports a spurious
  mismatch, which is the error this replaced. bv_decide then
  discharges the unfolded goal by compiling it to a SAT instance,
  refuting it with the bundled CaDiCaL solver, and checking the
  resulting proof in Lean's kernel -- not by literally enumerating
  256 cases the way the Criterion test does, though the guarantee
  covers the same domain.
-/
theorem parityXorTree_correct : ∀ b : BitVec 8, parityXorTree b = paritySpec b := by
  intro b
  simp only [parityXorTree, paritySpec]
  bv_decide