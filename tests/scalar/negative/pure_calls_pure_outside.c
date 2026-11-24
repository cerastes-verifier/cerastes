// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/scalar/pure_calls_pure.c
// Pins: 0062 items 7 and 9: a pure function that applies another outside the callee's requires: the precondition error, in the function, at the application, with a note at the callee's clause
// Pre-registered from record 0062 (Part B), before the code: precondition
#include <cerastes.h>

CERASTES_PURE int half(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r && r <= x)
{
  return x / 2;
}

CERASTES_PURE int halved(int x)
    CERASTES_REQUIRES(-5 < x)
{
  return half(x);
}

int use(int y)
    CERASTES_REQUIRES(0 <= y)
{
  CERASTES_CHECK(halved(y) == halved(y));
  return y;
}
