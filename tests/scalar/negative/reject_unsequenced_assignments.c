// expect: rejected cerastes-unsequenced_calls @ L29
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: two stores, each of a different call's result: the writes are items of the chain test, and their sets are not a chain
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

int test(int a, int b)
{
  int x;
  int y;
  int s = (x = f(a)) + (y = g(b));
  return s + x + y;
}
