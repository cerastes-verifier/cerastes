// expect: rejected cerastes-unsequenced_calls @ L27
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: the comma orders f before 100 / g(y), but 100 / h(z) depends on h alone: the sets do not form a chain
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
  return (f(x), 100 / g(y)) + 100 / h(z);
}
