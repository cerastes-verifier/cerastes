// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 item 3: 100 ^ 255 ^ 15 ^ 3 is 151 (closed, decided by evaluation); split from mask_or_xor_round_trip.c, one mask chain per file: queried one after another in one prover, these methods made Z3 run to the time limit on CI (prover history; M4.5b1a follow-up, record 0027 item 4)
#include <cerastes.h>

int xor_chain_value(void)
    CERASTES_ENSURES(r, r == 151)
{
  int x = 100;
  return ((x ^ 255) ^ 15) ^ 3;
}
