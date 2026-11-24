// expect: error check @ L17
// mode: c23, c++20
// twin: tests/scalar/pure_return_narrowing_ok.c
// Pins: 0062 item 7 (audit round 2 R2-S3): narrow(y) is y, so == y + 1 is the check error
// Pre-registered from record 0062 (Part B), before the code: check. the value-observing twin of the positive: one check error, at the check
#include <cerastes.h>

CERASTES_PURE short narrow(int x)
    CERASTES_REQUIRES(-100 < x && x < 100)
{
  return (short)x;
}

int use(int y)
    CERASTES_REQUIRES(-50 < y && y < 50)
{
  CERASTES_CHECK(narrow(y) == y + 1);
  return y;
}
