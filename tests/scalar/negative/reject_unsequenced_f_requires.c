// expect: rejected cerastes-unsequenced_calls @ L28
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: f(g(x)) + 100 / h(y) with a precondition on f's argument: the precondition of f needs g first, and the division needs h first
#include <cerastes.h>

int g(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int f(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int test(int x, int y)
{
  return f(g(x)) + 100 / h(y);
}
