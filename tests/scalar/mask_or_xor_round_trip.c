// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0027 items 1 and 4: applying the same constant mask twice gives x back (int, unsigned, a complemented mask); the deeper chains are mask_or_xor_chain.c, mask_or_xor_chain_value.c, mask_or_xor_chain4.c and mask_or_xor_chain8_range.c, one mask chain per file: queried one after another in one prover, these methods made Z3 run to the time limit on CI (prover history; M4.5b1a follow-up, record 0027 item 4)
#include <cerastes.h>

int xor_round_trip(int x)
    CERASTES_ENSURES(r, r == x)
{
  return (x ^ 255) ^ 255;
}

unsigned xor_round_trip_unsigned(unsigned u)
    CERASTES_ENSURES(r, r == u)
{
  return (u ^ 4294967295u) ^ 4294967295u;
}

int xor_round_trip_complement(int x)
    CERASTES_ENSURES(r, r == x)
{
  return (x ^ ~15) ^ ~15;
}
