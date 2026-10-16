/-
  The dir sequence of the Hilbert curve recursion (Figure 16-3).

  hilbertDirs n d r is the list of values hilbert(d, r, n) hands to step(),
  steps only (the starting point is not a step). Each level is four
  sub-curves with three single steps between them, which is why the proofs
  below go through join7: it names that shape, so every induction step turns
  into seven small goals (four sub-curves and three joints) instead of one
  large rewrite.

  What is proved, for every order n and every d and r:

    hilbertDirs_length   a curve takes 4^n - 1 steps
    hilbertDirs_rotate   starting k quarter turns further round adds k to
                         every step
    hilbertDirs_mirror   flipping the sign of r reflects every step across
                         the d axis: x becomes 2d - x
    hilbertDirs_reverse  walking the curve backwards, negating each step, is
                         the same recursion with d negated
    hilbertDirs_top_antipalindrome
                         at the top level (d = 0, r = 1) the sequence reads
                         the same backwards with each step negated: the
                         curve is left-right symmetric

  Not proved here: that a curve ends (2^n - 1) units from where it started,
  and that it visits every cell of a 2^n by 2^n square exactly once. Both
  need a notion of position and are separate, larger proofs.

  No Mathlib is needed. The tactics are induction, simp, rw and omega.
-/

/-- Four sub-curves a, b, c, e with a single step between each pair. -/
def join7 (a : List Int) (x : Int) (b : List Int) (y : Int)
    (c : List Int) (z : Int) (e : List Int) : List Int :=
  a ++ [x] ++ b ++ [y] ++ c ++ [z] ++ e

/-- The recursion from Figure 16-3, as the list of dir values passed to step(). -/
def hilbertDirs : Nat → Int → Int → List Int
  | 0, _, _ => []
  | n+1, d, r =>
      hilbertDirs n (d + r) (-r) ++ [d + r] ++
      hilbertDirs n d r ++ [d] ++
      hilbertDirs n d r ++ [d - r] ++
      hilbertDirs n (d - r) (-r)

/- ---------- Unfolding, and three small facts about join7 ---------- -/

theorem hilbertDirs_zero (d r : Int) : hilbertDirs 0 d r = [] := by
  first
    | rfl
    | simp [hilbertDirs]

/-- One level of the recursion, written with join7. -/
theorem hilbertDirs_succ (n : Nat) (d r : Int) :
    hilbertDirs (n + 1) d r =
      join7 (hilbertDirs n (d + r) (-r)) (d + r)
            (hilbertDirs n d r) d
            (hilbertDirs n d r) (d - r)
            (hilbertDirs n (d - r) (-r)) := by
  first
    | rfl
    | simp [hilbertDirs, join7]

/-- Applying a function to a join applies it to every piece. -/
theorem join7_map (f : Int → Int) (a : List Int) (x : Int) (b : List Int)
    (y : Int) (c : List Int) (z : Int) (e : List Int) :
    (join7 a x b y c z e).map f =
      join7 (a.map f) (f x) (b.map f) (f y) (c.map f) (f z) (e.map f) := by
  first
    | simp [join7]
    | simp [join7, List.map_append, List.append_assoc]

/-- Reversing a join reverses the order of the pieces and each piece. -/
theorem join7_reverse (a : List Int) (x : Int) (b : List Int) (y : Int)
    (c : List Int) (z : Int) (e : List Int) :
    (join7 a x b y c z e).reverse =
      join7 e.reverse z c.reverse y b.reverse x a.reverse := by
  first
    | simp [join7]
    | simp [join7, List.reverse_append, List.append_assoc]

/-- Two joins are equal when their pieces are. -/
theorem join7_congr {a a' : List Int} {x x' : Int} {b b' : List Int}
    {y y' : Int} {c c' : List Int} {z z' : Int} {e e' : List Int}
    (ha : a = a') (hx : x = x') (hb : b = b') (hy : y = y')
    (hc : c = c') (hz : z = z') (he : e = e') :
    join7 a x b y c z e = join7 a' x' b' y' c' z' e' := by
  subst ha hx hb hy hc hz he
  rfl

/- ---------- Length: 4^n - 1 steps ---------- -/

theorem hilbertDirs_length (n : Nat) :
    ∀ d r : Int, (hilbertDirs n d r).length + 1 = 4 ^ n := by
  induction n with
  | zero =>
    intro d r
    simp [hilbertDirs_zero]
  | succ n ih =>
    intro d r
    -- the four sub-curves each have 4^n - 1 steps, plus three joints
    have h1 := ih (d + r) (-r)
    have h2 := ih d r
    have h3 := ih (d - r) (-r)
    have h4 : 4 ^ (n + 1) = 4 ^ n * 4 := Nat.pow_succ 4 n
    rw [hilbertDirs_succ]
    simp only [join7, List.length_append, List.length_cons, List.length_nil]
    first
      | omega
      | (rw [Nat.pow_succ]; omega)

/- ---------- Rotation: a quarter turn adds 1 to every step ---------- -/

theorem hilbertDirs_rotate (n : Nat) :
    ∀ d r k : Int,
      hilbertDirs n (d + k) r = (hilbertDirs n d r).map (fun x => x + k) := by
  induction n with
  | zero =>
    intro d r k
    simp [hilbertDirs_zero]
  | succ n ih =>
    intro d r k
    first
      | rw [hilbertDirs_succ, hilbertDirs_succ, join7_map]
      | simp only [hilbertDirs_succ, join7_map]
    refine join7_congr ?_ ?_ ?_ ?_ ?_ ?_ ?_
    · -- first quarter: uses ih at (d + r, -r)
      exact (congrArg (fun t => hilbertDirs n t (-r))
              (by omega : d + k + r = d + r + k)).trans (ih (d + r) (-r) k)
    · -- first joint
      first
        | (show d + k + r = d + r + k; omega)
        | omega
    · exact ih d r k
    · -- middle joint
      first
        | rfl
        | (show d + k = d + k; rfl)
        | omega
    · exact ih d r k
    · -- last joint
      first
        | (show d + k - r = d - r + k; omega)
        | omega
    · -- last quarter: uses ih at (d - r, -r)
      exact (congrArg (fun t => hilbertDirs n t (-r))
              (by omega : d + k - r = d - r + k)).trans (ih (d - r) (-r) k)

/- ---------- Mirror: flipping r reflects every step across the d axis ---------- -/

/-- The general form: reflecting the direction as well as flipping the sign of r. -/
theorem hilbertDirs_flip (n : Nat) :
    ∀ d r b : Int,
      hilbertDirs n (-d + b) (-r) = (hilbertDirs n d r).map (fun x => -x + b) := by
  induction n with
  | zero =>
    intro d r b
    simp [hilbertDirs_zero]
  | succ n ih =>
    intro d r b
    first
      | rw [hilbertDirs_succ, hilbertDirs_succ, join7_map]
      | simp only [hilbertDirs_succ, join7_map]
    refine join7_congr ?_ ?_ ?_ ?_ ?_ ?_ ?_
    · -- first quarter: uses ih at (d + r, -r)
      exact (congrArg (fun t => hilbertDirs n t (-(-r)))
              (by omega : -d + b + -r = -(d + r) + b)).trans (ih (d + r) (-r) b)
    · first
        | (show -d + b + -r = -(d + r) + b; omega)
        | omega
    · exact ih d r b
    · first
        | rfl
        | (show -d + b = -d + b; rfl)
        | omega
    · exact ih d r b
    · first
        | (show -d + b - -r = -(d - r) + b; omega)
        | omega
    · -- last quarter: uses ih at (d - r, -r)
      exact (congrArg (fun t => hilbertDirs n t (-(-r)))
              (by omega : -d + b - -r = -(d - r) + b)).trans (ih (d - r) (-r) b)

/-- Flipping the sign of r reflects every step across the d axis: x becomes 2d - x. -/
theorem hilbertDirs_mirror (n : Nat) (d r : Int) :
    hilbertDirs n d (-r) = (hilbertDirs n d r).map (fun x => 2 * d - x) := by
  have h := hilbertDirs_flip n d r (2 * d)
  have e : -d + 2 * d = d := by omega
  have hf : (fun x : Int => -x + 2 * d) = (fun x : Int => 2 * d - x) := by
    funext x
    omega
  rw [e] at h
  exact h.trans (congrArg (fun f => (hilbertDirs n d r).map f) hf)

/- ---------- Reversal: walking backwards ---------- -/

theorem hilbertDirs_reverse (n : Nat) :
    ∀ d r : Int,
      (hilbertDirs n d r).reverse = (hilbertDirs n (-d) r).map (fun x => -x) := by
  induction n with
  | zero =>
    intro d r
    simp [hilbertDirs_zero]
  | succ n ih =>
    intro d r
    first
      | rw [hilbertDirs_succ, hilbertDirs_succ, join7_reverse, join7_map]
      | simp only [hilbertDirs_succ, join7_reverse, join7_map]
    refine join7_congr ?_ ?_ ?_ ?_ ?_ ?_ ?_
    · -- the last quarter, walked backwards, becomes the first: ih at (d - r, -r)
      exact (ih (d - r) (-r)).trans
        (congrArg (fun t => (hilbertDirs n t (-r)).map (fun x => -x))
          (by omega : -(d - r) = -d + r))
    · first
        | (show d - r = -(-d + r); omega)
        | omega
    · exact ih d r
    · first
        | (show d = -(-d); omega)
        | omega
    · exact ih d r
    · first
        | (show d + r = -(-d - r); omega)
        | omega
    · -- the first quarter, walked backwards, becomes the last: ih at (d + r, -r)
      exact (ih (d + r) (-r)).trans
        (congrArg (fun t => (hilbertDirs n t (-r)).map (fun x => -x))
          (by omega : -(d + r) = -d - r))

/-- At the top level, read backwards with each step negated, the sequence is
    unchanged: the curve is left-right symmetric. -/
theorem hilbertDirs_top_antipalindrome (n : Nat) :
    (hilbertDirs n 0 1).reverse = (hilbertDirs n 0 1).map (fun x => -x) := by
  have h := hilbertDirs_reverse n 0 1
  first
    | (rw [Int.neg_zero] at h; exact h)
    | simpa using h
