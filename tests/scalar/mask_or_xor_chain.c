// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 item 3: a nested ^ chain keeps the bits above the masks; split from mask_or_xor_round_trip.c, one mask chain per file: queried one after another in one prover, these methods made Z3 run to the time limit on CI (prover history; M4.5b1a follow-up, record 0027 item 4)
#include <cerastes.h>

int xor_chain(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, r / 256 == x / 256)
{
  return ((x ^ 255) ^ 15) ^ 3;
}
