// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 3, 4 and 7: a pure function with a value requires and an ensures; an application inside its requires is a precondition row of the check and the ensures is used
// Pre-registered from record 0062 (Part B), before the code: verifies
#include <cerastes.h>

CERASTES_PURE int half(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r && r <= x)
{
  return x / 2;
}

int use(int y)
    CERASTES_REQUIRES(0 <= y)
{
  CERASTES_CHECK(0 <= half(y));
  CERASTES_CHECK(half(y) <= y);
  return y;
}
