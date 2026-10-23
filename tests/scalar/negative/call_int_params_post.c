// expect: error postcondition @ L32
// mode: c23, c++20
// twin: tests/scalar/call_int_params.c
// Pins: 0021: the result is x - y - 1, not x - y
#include <cerastes.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int h(int y)
    CERASTES_REQUIRES(0 <= y && y < 2147483646)
    CERASTES_ENSURES(r, r == y + 2 && r >= 2)
{
  return y + 2;
}

int g(int a, int b)
    CERASTES_REQUIRES(1 <= a && 2 <= b)
    CERASTES_ENSURES(r, r == a - b && -b < r && r < a - 1)
{
  return a - b;
}

int test(int x, int y)
    CERASTES_REQUIRES(0 <= x && x < 2147483647 && 0 <= y && y < 2147483646)
    CERASTES_ENSURES(r, r == x - y)
{
  return g(f(x), h(y));
}
