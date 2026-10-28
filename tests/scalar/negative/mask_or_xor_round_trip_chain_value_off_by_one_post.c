// expect: error postcondition @ L36
// mode: c23, c++20
// twin: tests/scalar/mask_or_xor_chain_value.c
// Pins: 0027 item 3: 100 ^ 255 ^ 15 ^ 3 is 151 (closed, decided by evaluation)
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

int xor_chain_value(void)
    CERASTES_ENSURES(r, r == 150)
{
  int x = 100;
  return ((x ^ 255) ^ 15) ^ 3;
}
