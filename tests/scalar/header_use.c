// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0011 items 4 and 5: the body and the contract of a user header function are verified and used
// tests/scalar/headers/clamp0.h has a body, verified with this test, and a contract this test relies on.
#include <cerastes.h>

#include "headers/clamp0.h"

int use(int x)
    CERASTES_REQUIRES(x >= -1000 && x <= 1000)
    CERASTES_ENSURES(r, r >= 1 && r <= 1001)
{
  return clamp0(x) + 1;
}
