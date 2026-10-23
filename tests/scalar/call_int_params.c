// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0021: g(f(x), h(y)) with int parameters: no conversion depends on one call alone, so the sets form a chain (the short version is rejected)
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
    CERASTES_REQUIRES(-2 <= x && x < 2147483647 && -2 <= y && y < 2147483646)
    CERASTES_ENSURES(r, r == x - (y + 1))
{
  return g(f(x), h(y));
}
