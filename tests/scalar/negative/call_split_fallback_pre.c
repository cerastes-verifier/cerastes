// expect: error precondition @ L30
// mode: c23, c++20
// twin: tests/scalar/call_split_fallback.c
// Pins: 0026 item 2: the fallback still checks h's requires: y is unconstrained from below, so 0 < b fails at the call
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int g(int z)
    CERASTES_ENSURES(r, r == 1)
{
  return 1 + (z - z);
}

int h(int a, int b)
    CERASTES_REQUIRES(0 < b)
    CERASTES_ENSURES(r, r == b)
{
  return b + (a - a);
}

int test(int x, int y, int z)
    CERASTES_REQUIRES(y <= 2147483547)
    CERASTES_ENSURES(r, r == y + 100)
{
  return h(f(x), y) + 100 / g(z);
}
