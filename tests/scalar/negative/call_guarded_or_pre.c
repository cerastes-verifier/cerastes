// expect: error precondition @ L17
// mode: c23, c++20
// twin: tests/scalar/call_guarded_or.c
// Pins: 0021: the guard a < 0 lets a == 0 reach the call
#include <cerastes.h>

int pos(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x && r > 0)
{
  return x;
}

int guarded_or(int a)
    CERASTES_ENSURES(r, r == (a != 1))
{
  return a < 0 || pos(a) > 1;
}
