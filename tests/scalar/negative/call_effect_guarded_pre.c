// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/scalar/call_effect_guarded.c
// Pins: 0021: the guard a >= 0 lets a == 0 reach the call
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int f(int a)
    CERASTES_ENSURES(r, r == (a > 5 ? a : 0))
{
  int x;
  if (a >= 0 && (x = pos(a)) > 5) {
    return x;
  }
  return 0;
}
