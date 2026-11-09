import Lake
open Lake DSL

package proofs where

@[default_target]
lean_lib Proofs

require mathlib from git
  "https://github.com/leanprover-community/mathlib4" @ "v4.35.0"
