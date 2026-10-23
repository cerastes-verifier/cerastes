// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: f(g(x)) + 100 / h(y): the division is checked right after h, before g and f, which no check depends on (the order is h, g, f)
#include <cerastes.h>

int g(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int f(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int test(int x, int y)
    CERASTES_REQUIRES(0 < y && x <= 2147483647 - 100 / y)
    CERASTES_ENSURES(r, r == x + 100 / y)
{
  return f(g(x)) + 100 / h(y);
}
