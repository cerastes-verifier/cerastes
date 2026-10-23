// expect: rejected cerastes-unsequenced_calls @ L29
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: g(f(x), h(y)) where the contract of the callee is split into one requires per argument: each clause depends on one call alone
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int z)
    CERASTES_ENSURES(r, r == z)
{
  return z;
}

int take(int a, int b)
    CERASTES_REQUIRES(1 <= a)
    CERASTES_REQUIRES(2 <= b)
    CERASTES_ENSURES(r, r == a - b)
{
  return a - b;
}

int test(int x, int y)
{
  return take(f(x), h(y));
}
