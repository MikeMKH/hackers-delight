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
