// expect: error check @ L16
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 7: the fact of a check over an application that the facts do not imply is the check error, at the check
// Pre-registered from record 0062 (Part B), before the code: check
#include <cerastes.h>

CERASTES_PURE bool is_small(int x)
{
  return -100 < x && x < 100;
}

int use(int y)
    CERASTES_REQUIRES(-150 < y && y < 150)
{
  CERASTES_CHECK(is_small(y));
  return y;
}
