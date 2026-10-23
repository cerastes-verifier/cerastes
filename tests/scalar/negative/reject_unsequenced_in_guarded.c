// expect: rejected cerastes-unsequenced_calls @ L27
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: the unsequenced pair inside a guarded operand is rejected on its own, by the branch's own flush
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

int test(int a, int x, int y)
{
  return a && (100 / f(x) + 100 / g(y) > 0);
}
