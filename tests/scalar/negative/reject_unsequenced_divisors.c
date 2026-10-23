// expect: rejected cerastes-unsequenced_calls @ L27
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: two divisions, each depending on a different call: the checks cannot all follow the calls they depend on, and a call that never returns would hide one of them
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

int test(int x, int y)
{
  return 100 / f(x) + 100 / g(y);
}
