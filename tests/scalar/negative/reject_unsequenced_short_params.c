// expect: rejected cerastes-unsequenced_calls @ L26
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: a conversion to short of each result depends on one call alone: the sets {f} and {h} are not a chain
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

int take(short a, short b)
{
  return a + b;
}

int test(int x, int y)
{
  return take(f(x), h(y));
}
