// expect: error shift @ L17
// mode: c23, c++20
// twin: tests/scalar/shr_floor.c
// Pins: 0010 item 8: a right shift by 32 is out of range too
#include <cerastes.h>

int half_floor(int a)
    CERASTES_REQUIRES(a == -7)
    CERASTES_ENSURES(r, r == -4)
{
  return a >> 1;
}

int shr_var(int a, int n)
    CERASTES_REQUIRES(n >= 0 && n <= 32)
{
  return a >> n;
}
