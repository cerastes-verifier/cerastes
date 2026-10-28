// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 item 4: four masks that cancel in adjacent pairs give x back; split from mask_or_xor_round_trip.c, one mask chain per file: queried one after another in one prover, these methods made Z3 run to the time limit on CI (prover history; M4.5b1a follow-up, record 0027 item 4)
#include <cerastes.h>

int xor_chain4(int x)
    CERASTES_ENSURES(r, r == x)
{
  return (((x ^ 255) ^ 255) ^ 15) ^ 15;
}
