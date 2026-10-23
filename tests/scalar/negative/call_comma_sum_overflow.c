// expect: error overflow @ L25
// mode: c23, c++20
// twin: tests/scalar/call_comma_sum.c
// Pins: 0021: y may be INT_MAX - 1, so two(y) + 1 overflows
#include <cerastes.h>

int one(int x)
    CERASTES_REQUIRES(0 <= x && x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1 && r > 0)
{
  return x + 1;
}

int two(int y)
    CERASTES_REQUIRES(0 <= y && y < 2147483647)
    CERASTES_ENSURES(r, r == y + 1 && r > 0)
{
  return y + 1;
}

int test(int x, int y)
    CERASTES_REQUIRES(0 <= x && x < 2147483647 && 0 <= y && y < 2147483647)
    CERASTES_ENSURES(r, r > y)
{
  return (one(x), 1) + two(y);
}
