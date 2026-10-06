import Std.Tactic.BVDecide

/-
  GF(2), the two-element field: {O, I} under XOR for addition and AND
  for multiplication. Represented as BitVec 1.

  O and I are the classic abstract-algebra additive and multiplicative
  identity. In GF(2), and only in GF(2), O and I happen to coincide
  exactly with bits 0 and 1.
-/

abbrev GF2 := BitVec 1

def gf2Add (a b : GF2) : GF2 := a ^^^ b
def gf2Mul (a b : GF2) : GF2 := a &&& b

/-- Additive identity. -/
def O : GF2 := 0#1
/-- Multiplicative identity. -/
def I : GF2 := 1#1

/--
  A field's two identity elements must be distinct -- this is what
  makes it a genuine (nontrivial) field rather than the degenerate
  one-element ring where O = I.
-/
theorem O_ne_I : O ≠ I := by
  simp only [O, I]
  bv_decide

/- (GF2, +) is an abelian group. -/

theorem gf2Add_comm : ∀ a b : GF2, gf2Add a b = gf2Add b a := by
  intro a b
  simp only [gf2Add]
  bv_decide

theorem gf2Add_assoc : ∀ a b c : GF2, gf2Add (gf2Add a b) c = gf2Add a (gf2Add b c) := by
  intro a b c
  simp only [gf2Add]
  bv_decide

theorem gf2Add_identity : ∀ a : GF2, gf2Add a O = a := by
  intro a
  simp only [gf2Add, O]
  bv_decide

/--
  Characteristic 2: every element is its own additive inverse. There
  is no separate "subtract" in GF(2) -- XOR undoes itself -- which is
  exactly why subtraction and XOR were interchangeable throughout the
  GF(2) division work.
-/
theorem gf2Add_self_inverse : ∀ a : GF2, gf2Add a a = O := by
  intro a
  simp only [gf2Add, O]
  bv_decide

/-
  (GF2 \ {O}, *) is an abelian group -- here just the single element
  {I}, since GF2 only has two elements total.
-/

theorem gf2Mul_comm : ∀ a b : GF2, gf2Mul a b = gf2Mul b a := by
  intro a b
  simp only [gf2Mul]
  bv_decide

theorem gf2Mul_assoc : ∀ a b c : GF2, gf2Mul (gf2Mul a b) c = gf2Mul a (gf2Mul b c) := by
  intro a b c
  simp only [gf2Mul]
  bv_decide

theorem gf2Mul_identity : ∀ a : GF2, gf2Mul a I = a := by
  intro a
  simp only [gf2Mul, I]
  bv_decide

/-- The only nonzero element is its own multiplicative inverse. -/
theorem I_self_inverse : gf2Mul I I = I := by
  simp only [gf2Mul, I]
  bv_decide

/-
  Distributivity -- the law that actually links + and * into a
  single field, rather than leaving them as two unrelated groups.
-/
theorem gf2_distrib : ∀ a b c : GF2, gf2Mul a (gf2Add b c) = gf2Add (gf2Mul a b) (gf2Mul a c) := by
  intro a b c
  simp only [gf2Mul, gf2Add]
  bv_decide
