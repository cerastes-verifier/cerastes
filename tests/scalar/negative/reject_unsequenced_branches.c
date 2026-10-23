// expect: rejected cerastes-unsequenced_calls @ L27
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: two branches with calls: the second branch's checks would be hidden by the first branch's call
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

int test(int a, int b, int x, int y)
{
  return (a && g(x)) + (b && h(y));
}
