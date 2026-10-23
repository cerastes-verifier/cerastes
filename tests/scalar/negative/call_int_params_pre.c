// expect: error precondition @ L32
// mode: c23, c++20
// twin: tests/scalar/call_int_params.c
// Pins: 0021: x may be INT_MAX, the precondition of f is checked
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int h(int y)
    CERASTES_REQUIRES(y < 2147483646)
    CERASTES_ENSURES(r, r == y + 2)
{
  return y + 2;
}

int g(int a, int b)
    CERASTES_REQUIRES(-1 <= a && 0 <= b)
    CERASTES_ENSURES(r, r == a - b)
{
  return a - b;
}

int test(int x, int y)
    CERASTES_REQUIRES(-2 <= x && x <= 2147483647 && 0 <= y && y < 2147483646)
    CERASTES_ENSURES(r, r == x - (y + 1))
{
  return g(f(x), h(y));
}
