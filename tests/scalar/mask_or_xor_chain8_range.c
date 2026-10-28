// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 item 4: an eight-mask chain keeps a non-negative x non-negative (a range claim: an identity through a deep chain may be beyond what Z3 decides); split from mask_or_xor_round_trip.c, one mask chain per file: queried one after another in one prover, these methods made Z3 run to the time limit on CI (prover history; M4.5b1a follow-up, record 0027 item 4)
#include <cerastes.h>

int xor_chain8_range(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r)
{
  return ((((((((x ^ 255) ^ 15) ^ 255) ^ 15) ^ 255) ^ 15) ^ 255) ^ 15);
}
