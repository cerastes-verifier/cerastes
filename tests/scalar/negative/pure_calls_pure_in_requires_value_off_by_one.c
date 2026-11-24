// expect: error check @ L23
// mode: c23, c++20
// twin: tests/scalar/pure_calls_pure_in_requires.c
// Pins: 0062 item 7 (audit round 2 R2-S3): pred(y) is y - 1, so == y is the check error
// Pre-registered from record 0062 (Part B), before the code: check. the value-observing twin of the positive: one check error, at the check
#include <cerastes.h>

CERASTES_PURE bool positive(int x)
    CERASTES_ENSURES(r, r == (0 < x))
{
  return 0 < x;
}

CERASTES_PURE int pred(int x)
    CERASTES_REQUIRES(positive(x))
{
  return x - 1;
}

int use(int y)
    CERASTES_REQUIRES(0 < y)
{
  CERASTES_CHECK(pred(y) == y);
  return y;
}
