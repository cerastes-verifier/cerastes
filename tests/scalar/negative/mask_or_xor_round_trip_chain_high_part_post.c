// expect: error postcondition @ L29
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_chain.c
// Pins: 0027 item 3: the chain keeps the bits above the masks, so r / 256 is x / 256, not one more
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

int xor_chain(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r / 256 == x / 256 + 1)
{
  return ((x ^ 255) ^ 15) ^ 3;
}
