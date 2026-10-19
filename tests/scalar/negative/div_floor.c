// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/div_trunc.c
// Pins: 0010 item 7: -7 / 2 is -3, not the floor -4
#include <cerastes.h>

int quot(int a, int b)
    CERASTES_REQUIRES(a == -7 && b == 2)
    CERASTES_ENSURES(r, r == -4)
{
  return a / b;
}

int rem(int a, int b)
    CERASTES_REQUIRES(a == -7 && b == 2)
    CERASTES_ENSURES(r, r == -1)
{
  return a % b;
}
