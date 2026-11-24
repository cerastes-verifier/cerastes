// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/scalar/pure_body_overflow_excluded.c
// Pins: 0062 item 9: an operation of a pure body that may overflow is the overflow error, in the function, at the operator (whoever calls it)
// Pre-registered from record 0062 (Part B), before the code: overflow
#include <cerastes.h>

CERASTES_PURE int sq(int x)
{
  return x * x;
}

int use(int y)
    CERASTES_REQUIRES(-100 < y && y < 100)
{
  CERASTES_CHECK(sq(y) == sq(y));
  return y;
}
