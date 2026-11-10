import Mathlib

open Polynomial

/- f(n) = n² + n + 41 -/
def euler (n : ℤ) : ℤ := n ^ 2 + n + 41

/- g(n) = f(n - 40) = n² - 79n + 1601 -/
def g (n : ℤ) : ℤ := n ^ 2 - 79 * n + 1601

/- f(n) is prime for 0 ≤ n ≤ 39 -/
theorem euler_prime (n : ℕ) (hn : n < 40) : Nat.Prime (n ^ 2 + n + 41) := by
  interval_cases n <;> norm_num

/- f(-n) = n² - n + 41 = f(n - 1), so f is prime for n = -1 .. -40 -/
theorem euler_neg (n : ℤ) : euler (-n) = euler (n - 1) := by
  unfold euler; ring

/- f(n - 40) = n² - 79n + 1601 -/
theorem euler_shift (n : ℤ) : euler (n - 40) = g n := by
  unfold euler g; ring

/- g is prime-valued for 0 ≤ n ≤ 79 -/
theorem g_prime (n : ℕ) (hn : n < 80) : Nat.Prime (g n).natAbs := by
  interval_cases n <;> norm_num [g]

/- nonmonotonic, repeating shape: symmetric about 39.5 -/
theorem g_symm (n : ℤ) : g n = g (79 - n) := by
  unfold g; ring

example : g 0 = 1601 ∧ g 1 = 1523 ∧ g 39 = 41 ∧ g 40 = 41 ∧ g 79 = 1601 := by
  norm_num [g]

theorem exists_nonprime_value (f : ℤ[X]) (hf : 0 < f.natDegree) :
    ∃ n : ℕ, ¬ Nat.Prime (f.eval (n : ℤ)).natAbs := by
  by_contra hcon
  push Not at hcon
  -- p = |f(0)| is prime
  obtain ⟨p, hpdef⟩ : ∃ p : ℕ, p = (f.eval (0 : ℤ)).natAbs := ⟨_, rfl⟩
  have hp : p.Prime := by rw [hpdef]; simpa using hcon 0
  -- p ∣ f(k·p) for every k
  have hdiv : ∀ k : ℕ, (p : ℤ) ∣ f.eval (((k * p : ℕ)) : ℤ) := by
    intro k
    have h1 := Polynomial.sub_dvd_eval_sub (((k * p : ℕ)) : ℤ) 0 f
    have h2 : (p : ℤ) ∣ ((k * p : ℕ) : ℤ) - 0 := ⟨(k : ℤ), by push_cast; ring⟩
    have h3 : (p : ℤ) ∣ f.eval (0 : ℤ) := by
      rw [hpdef]; exact Int.natAbs_dvd.mpr (dvd_refl _)
    have h5 := dvd_add (dvd_trans h2 h1) h3
    rwa [sub_add_cancel] at h5
  -- f(k·p) is prime and divisible by the prime p, so |f(k·p)| = p
  have hval : ∀ k : ℕ, f.eval (((k * p : ℕ)) : ℤ) = p ∨ f.eval (((k * p : ℕ)) : ℤ) = -p := by
    intro k
    have hq := hcon (k * p)
    have hpq : p ∣ (f.eval (((k * p : ℕ)) : ℤ)).natAbs :=
      Int.natAbs_dvd_natAbs.mpr (hdiv k)
    exact Int.natAbs_eq_iff.mp ((Nat.prime_dvd_prime_iff_eq hp hq).mp hpq).symm
  -- ... so (f - p)(f + p) has infinitely many roots, forcing it to be zero
  have hne1 : f - C (p : ℤ) ≠ 0 := by
    intro h
    have : f = C (p : ℤ) := sub_eq_zero.mp h
    rw [this] at hf; simp at hf
  have hne2 : f + C (p : ℤ) ≠ 0 := by
    intro h
    have : f = -C (p : ℤ) := eq_neg_of_add_eq_zero_left h
    rw [this] at hf; simp at hf
  have hne : (f - C (p : ℤ)) * (f + C (p : ℤ)) ≠ 0 := mul_ne_zero hne1 hne2
  have hroot : ∀ x : ℤ, (f.eval x = p ∨ f.eval x = -p) →
      x ∈ (((f - C (p : ℤ)) * (f + C (p : ℤ))).roots.toFinset : Set ℤ) := by
    intro x hx
    simp only [Finset.mem_coe, Multiset.mem_toFinset, mem_roots hne, IsRoot,
      eval_mul, eval_sub, eval_add, eval_C]
    rcases hx with h | h <;> simp [h]
  have hinj : Function.Injective (fun k : ℕ => (((k * p : ℕ)) : ℤ)) := by
    intro a b h
    exact Nat.eq_of_mul_eq_mul_right hp.pos (Nat.cast_injective h)
  have hinf := Set.infinite_of_injective_forall_mem hinj (fun k => hroot _ (hval k))
  exact hinf (Finset.finite_toSet _)
