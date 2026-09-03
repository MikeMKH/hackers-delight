#ifndef CHAPTER10_DIVIDE_BY_3_H
#define CHAPTER10_DIVIDE_BY_3_H

#include <stddef.h>
#include <stdint.h>

/* 
  Bit-serial divide-by-3 circuit (Hacker's Delight, Sec 10-23, Fig 10-50).
  Adapted from 32-bit to 8-bit: scans the dividend from MSB (bit 7) to
  LSB (bit 0), threading remainder state (r, s) through each stage
  exactly as the hardware circuit would, one bit per "clock cycle".
 */

typedef struct {
  uint8_t r;  /* remainder state bit r_i */
  uint8_t s;  /* remainder state bit s_i */
} div3_state_t;

/* 
  One combinational stage of the circuit: given the incoming state
  (r_{i+1}, s_{i+1}) and dividend bit x_i, produces quotient bit y_i
  and the outgoing state (r_i, s_i). Pure boolean logic -- no loops,
  no arithmetic -- mirroring Figure 10-50 gate-for-gate:
 
   y_i = r_{i+1} + s_{i+1} x_i
   r_i = !r_{i+1} s_{i+1} !x_i + r_{i+1} x_i
   s_i = !r_{i+1} !s_{i+1} x_i + r_{i+1} !s_{i+1} !x_i
 */
void div3_stage(div3_state_t in, uint8_t x_i, uint8_t *y_i, div3_state_t *out);

/* 
  Runs the full 8-bit division by threading div3_stage across all 8
  bits, MSB first. Initial state is r_8 = s_8 = 0. Final remainder is
  2*r_0 + s_0, exactly as in the book.
*/
uint8_t divide_by_3(uint8_t dividend, uint8_t *remainder);

/* 
  Same as divide_by_3, but also records the state after each stage
  (trace[0] = after bit 7, ... trace[7] = after bit 0) so callers can
  inspect or print the circuit's cycle-by-cycle behavior. trace must
  point to an array of at least 8 div3_state_t, or be NULL.
*/
uint8_t divide_by_3_traced(uint8_t dividend, uint8_t *remainder, div3_state_t *trace);
 
/* 
  Runs the division and prints a cycle-by-cycle trace of every stage
  to stdout: the incoming state, the dividend bit consumed, and the
  resulting quotient bit and outgoing state -- effectively probing
  the wires between each box in Figure 10-50 as it runs.
*/
void divide_by_3_print_trace(uint8_t dividend);

#endif