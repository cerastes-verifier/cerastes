// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0026 item 1: the guard (nonzero(d), 1) of && carries its floor, so the division it guards is checked after nonzero, which ends the program when d == 0
#include <cerastes.h>

// Returns only when d != 0 (a trusted contract: the callee ends the program otherwise).
int nonzero(int d)
    CERASTES_ENSURES(r, d != 0);

int guard(int d)
    CERASTES_ENSURES(r, d == 0 || r == (100 / d != 0))
{
  return (nonzero(d), 1) && 100 / d;
}
