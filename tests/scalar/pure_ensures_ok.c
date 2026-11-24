// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 2 and 9: a pure function with a contract that nothing calls is read and checked (its ensures holds)
// Pre-registered from record 0062 (Part B), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int dbl(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
    CERASTES_ENSURES(r, r == x + x)
{
  return x + x;
}
