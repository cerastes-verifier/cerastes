// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/scalar/pure_requires_value.c
// Pins: 0062 item 7: a check that applies a pure function outside its requires is the precondition error at the check (a note at the callee's clause)
// Pre-registered from record 0062 (Part B), before the code: precondition
#include <cerastes.h>

CERASTES_PURE int half(int x)
    CERASTES_REQUIRES(0 <= x)
    CERASTES_ENSURES(r, 0 <= r && r <= x)
{
  return x / 2;
}

int use(int y)
    CERASTES_REQUIRES(-5 < y)
{
  CERASTES_CHECK(half(y) <= y);
  return y;
}
