// expect: error precondition @ L17
// mode: c23, c++20
// twin: tests/scalar/uninit_guarded_comma.c
// Pins: 0025 item 1: with x == 1 the guard is true for a positive a, and g accepts only 0: the guard decides the call's precondition
#include <cerastes.h>

int g(int v)
    CERASTES_REQUIRES(v == 0)
    CERASTES_ENSURES(r, r == v);

int f(int a)
    CERASTES_ENSURES(r, r == a);

int initialized(int a)
{
  int x = 1;
  int y = (x = (g(f(a) > 0 && x > 0), 2));
  return y + x;
}
