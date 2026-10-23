// expect: error precondition @ L25
// mode: c23, c++20
// twin: tests/scalar/call_comma_sum.c
// Pins: 0021: the call before the comma is checked: x may be INT_MAX
#include <cerastes.h>

int one(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int two(int y)
    CERASTES_REQUIRES(y < 2147483647)
    CERASTES_ENSURES(r, r == y + 1)
{
  return y + 1;
}

int test(int x, int y)
    CERASTES_REQUIRES(x <= 2147483647 && y < 2147483646)
    CERASTES_ENSURES(r, r == y + 2)
{
  return (one(x), 1) + two(y);
}

int two_edge(void)
    CERASTES_ENSURES(r, r == 2147483647)
{
  return two(2147483646);
}
