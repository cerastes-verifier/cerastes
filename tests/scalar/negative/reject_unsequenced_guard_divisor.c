// expect: rejected cerastes-unsequenced_calls @ L27
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: a branch whose condition has a call, and a division by another call's result
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int g(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int h(int z)
    CERASTES_ENSURES(r, r == z)
{
  return z;
}

int test(int x, int y, int z)
{
  return (f(x) > 0 && 100 / y > 0) + 100 / g(z);
}
