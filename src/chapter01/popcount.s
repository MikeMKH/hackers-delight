.section __TEXT,__text
.global _popcount_asm
.align 2

// int popcount_asm(uint32_t x)
// Arguments: 
//   w0 := x
// Returns:
//   w0 := popcount(x)
_popcount_asm:
  fmov s0, w0       // move integer to SIMD register
  cnt  v0.8b, v0.8b // count set bits in each byte
  addv b0, v0.8b    // sum all bytes
  fmov w0, s0       // move result back to integer register
  ret