// expect: error postcondition @ L49
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_chain8_range.c
// Pins: 0027 item 4: the eight masks cancel in pairs, so r is x and r < x is false for every x (x == 0)
#include <cerastes.h>

// The other functions of mask_or_xor_round_trip.c are not repeated here: one mask chain per file: queried one after another in one prover, these methods made Z3 run to the time limit on CI (prover history; M4.5b1a follow-up, record 0027 item 4). These comment lines keep the line numbers.
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//

int xor_chain8_range(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r < x)
{
  return ((((((((x ^ 255) ^ 15) ^ 255) ^ 15) ^ 255) ^ 15) ^ 255) ^ 15);
}
