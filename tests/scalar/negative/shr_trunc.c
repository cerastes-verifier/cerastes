// expect: error postcondition @ L11
// mode: c23, c++20
// twin: tests/scalar/shr_floor.c
// Pins: 0010 item 8: -7 >> 1 is -4, not the truncated -3
#include <cerastes.h>

int half_floor(int a)
    CERASTES_REQUIRES(a == -7)
    CERASTES_ENSURES(r, r == -3)
{
  return a >> 1;
}

int shr_var(int a, int n)
    CERASTES_REQUIRES(n >= 0 && n <= 31)
{
  return a >> n;
}
