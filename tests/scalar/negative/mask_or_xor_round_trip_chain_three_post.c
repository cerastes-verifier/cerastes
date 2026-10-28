// expect: error postcondition @ L42
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_chain4.c
// Pins: 0027 item 4: with the last mask left out one ^ 15 is unpaired, so the result is x ^ 15, not x (x == 0 gives 15)
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

int xor_chain4(int x)
    CERASTES_ENSURES(r, r == x)
{
  return ((x ^ 255) ^ 255) ^ 15;
}
