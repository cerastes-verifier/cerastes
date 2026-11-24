// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 2, 4 and 7: a non-recursive pure function with no contract, named only by a check, is read and its application is its value
// Pre-registered from record 0062 (Part B), before the code: verifies
#include <cerastes.h>

CERASTES_PURE bool is_small(int x)
{
  return -100 < x && x < 100;
}

int use(int y)
    CERASTES_REQUIRES(-50 < y && y < 50)
{
  CERASTES_CHECK(is_small(y));
  return y;
}
