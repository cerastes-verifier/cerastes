// expect: error precondition @ L17
// mode: c23, c++20
// twin: tests/scalar/call_guarded_and.c
// Pins: 0021: the call before its guard: the precondition is checked without the guard (the swap-logical mutant)
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int guarded(int a)
    CERASTES_ENSURES(r, r == (a > 1))
{
  return pos(a) > 1 && a > 0;
}
