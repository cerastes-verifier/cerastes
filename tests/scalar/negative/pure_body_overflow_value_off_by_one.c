// expect: error check @ L17
// mode: c23, c++20
// twin: tests/scalar/pure_body_overflow_excluded.c
// Pins: 0062 item 7 (audit round 2 R2-S3): sq(y) is y * y, so == y * y + 1 is the check error
// Pre-registered from record 0062 (Part B), before the code: check. the value-observing twin of the positive: one check error, at the check
#include <cerastes.h>

CERASTES_PURE int sq(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
{
  return x * x;
}

int use(int y)
    CERASTES_REQUIRES(-100 < y && y < 100)
{
  CERASTES_CHECK(sq(y) == y * y + 1);
  return y;
}
