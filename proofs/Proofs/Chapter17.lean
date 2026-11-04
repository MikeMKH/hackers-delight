import Std.Tactic.BVDecide

def mag (x : BitVec 32) : BitVec 32 := x &&& 0x7FFFFFFF#32

/- IEEE: -0.0 = +0.0 -/
def ieeeLt (a b : BitVec 32) : Bool :=
  if !a.msb && !b.msb then (mag a).ult (mag b)
  else if a.msb && b.msb then (mag b).ult (mag a)
  else if a.msb && !b.msb then !(mag a == 0 && mag b == 0)
  else false

def ieeeEq (a b : BitVec 32) : Bool :=
  a == b || (mag a == 0 && mag b == 0)

/- non-IEEE: -0.0 < +0.0 -/
def nonIeeeLt (a b : BitVec 32) : Bool :=
  if !a.msb && !b.msb then (mag a).ult (mag b)
  else if a.msb && b.msb then (mag b).ult (mag a)
  else a.msb && !b.msb

/- Table 17-3, left column -/
def ieee1 (n : BitVec 32) : BitVec 32 :=
  if n.slt 0 then -n else n + 0x80000000#32
def ieee2 (n : BitVec 32) : BitVec 32 :=
  if n.slt 0 then (n ^^^ 0x7FFFFFFF#32) + 1 else n
def ieee3 (n : BitVec 32) : BitVec 32 :=
  if n.slt 0 then 0x80000000#32 - n else n
def ieee4 (n : BitVec 32) : BitVec 32 :=
  let t := n.sshiftRight 31
  (n ^^^ (t >>> 1)) - t          -- >>> is the logical shift

/- Table 17-3, right column -/
def nonieee1 (n : BitVec 32) : BitVec 32 :=
  if n.slt 0 then ~~~n else n + 0x80000000#32
def nonieee2 (n : BitVec 32) : BitVec 32 :=
  if n.slt 0 then n ^^^ 0x7FFFFFFF#32 else n
def nonieee3 (n : BitVec 32) : BitVec 32 :=
  if n.slt 0 then 0x7FFFFFFF#32 - n else n
def nonieee4 (n : BitVec 32) : BitVec 32 :=
  let t := (n.sshiftRight 30) >>> 1
  n ^^^ t

/- Rows that use unsigned comparison -/
theorem ieee1_correct (a b : BitVec 32) :
    ieeeLt a b = (ieee1 a).ult (ieee1 b) ∧
    ieeeEq a b = (ieee1 a == ieee1 b) := by
  unfold ieeeLt ieeeEq ieee1 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

theorem nonieee1_correct (a b : BitVec 32) :
    nonIeeeLt a b = (nonieee1 a).ult (nonieee1 b) ∧
    (a == b) = (nonieee1 a == nonieee1 b) := by
  unfold nonIeeeLt nonieee1 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

/- Rows that use signed comparison -/
theorem ieee2_correct (a b : BitVec 32) :
    ieeeLt a b = (ieee2 a).slt (ieee2 b) ∧
    ieeeEq a b = (ieee2 a == ieee2 b) := by
  unfold ieeeLt ieeeEq ieee2 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

theorem ieee3_correct (a b : BitVec 32) :
    ieeeLt a b = (ieee3 a).slt (ieee3 b) ∧
    ieeeEq a b = (ieee3 a == ieee3 b) := by
  unfold ieeeLt ieeeEq ieee3 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

theorem ieee4_correct (a b : BitVec 32) :
    ieeeLt a b = (ieee4 a).slt (ieee4 b) ∧
    ieeeEq a b = (ieee4 a == ieee4 b) := by
  unfold ieeeLt ieeeEq ieee4 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

theorem nonieee2_correct (a b : BitVec 32) :
    nonIeeeLt a b = (nonieee2 a).slt (nonieee2 b) ∧
    (a == b) = (nonieee2 a == nonieee2 b) := by
  unfold nonIeeeLt nonieee2 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

theorem nonieee3_correct (a b : BitVec 32) :
    nonIeeeLt a b = (nonieee3 a).slt (nonieee3 b) ∧
    (a == b) = (nonieee3 a == nonieee3 b) := by
  unfold nonIeeeLt nonieee3 mag
  refine ⟨?_, ?_⟩ <;> bv_decide

theorem nonieee4_correct (a b : BitVec 32) :
    nonIeeeLt a b = (nonieee4 a).slt (nonieee4 b) ∧
    (a == b) = (nonieee4 a == nonieee4 b) := by
  unfold nonIeeeLt nonieee4 mag
  refine ⟨?_, ?_⟩ <;> bv_decide
