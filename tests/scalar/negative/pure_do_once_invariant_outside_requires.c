// expect: error precondition @ L19
// mode: c23, c++20
// twin: tests/scalar/pure_do_once_invariant.c
// Pins: 0062 item 4 (owner question 284): an application outside the callee's requires in a do-once invariant is the precondition error at the invariant (the fact itself holds: sq(y) is y * y wherever it is defined)
// Pre-registered from question 284, before the code: precondition
#include <cerastes.h>

CERASTES_PURE int sq(int x)
    CERASTES_REQUIRES(-1000 < x && x < 1000)
{
  return x * x;
}

int f(int y)
    CERASTES_REQUIRES(-2000 < y && y < 2000)
    CERASTES_ENSURES(r, r == y * y)
{
  int s = 0;
  CERASTES_INVARIANT(sq(y) == y * y)
  do {
    s = y * y;
  } while (0);
  return s;
}
