// expect: error invariant-entry @ L19
// mode: c23, c++20
// twin: tests/scalar/pure_do_once_invariant.c
// Pins: 0062 item 4 (owner question 284): a false fact about a pure function in a do-once invariant is the invariant-entry error at the invariant (the callee's requires holds: no other row fails)
// Pre-registered from question 284, before the code: invariant-entry
#include <cerastes.h>

CERASTES_PURE int sq(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
{
  return x * x;
}

int f(int y)
    CERASTES_REQUIRES(-10 < y && y < 10)
    CERASTES_ENSURES(r, r == y * y)
{
  int s = 0;
  CERASTES_INVARIANT(sq(y) == y * y + 1)
  do {
    s = y * y;
  } while (0);
  return s;
}
