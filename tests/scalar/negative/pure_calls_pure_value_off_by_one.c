// expect: error check @ L24
// mode: c23, c++20
// twin: tests/scalar/pure_calls_pure.c
// Pins: 0062 item 7 (audit round 2 R2-S3): halved(y) is y / 2, so == y / 2 + 1 is the check error
// Pre-registered from record 0062 (Part B), before the code: check. the value-observing twin of the positive: one check error, at the check
#include <cerastes.h>

CERASTES_PURE int half(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r && r <= x)
{
  return x / 2;
}

CERASTES_PURE int halved(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return half(x);
}

int use(int y)
    CERASTES_REQUIRES(0 <= y)
{
  CERASTES_CHECK(halved(y) == y / 2 + 1);
  return y;
}
