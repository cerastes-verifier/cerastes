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
    CERASTES_ENSURES(r, r == 1)
{
  return 1 + (y - y);
}

int test(int x, int y)
    CERASTES_REQUIRES(x <= 2147483547)
    CERASTES_ENSURES(r, r == x + 100)
{
  return f(g(x)) + 100 / h(y);
}
