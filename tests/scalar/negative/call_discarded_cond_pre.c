// expect: error precondition @ L25
// mode: c23, c++20
// twin: tests/scalar/call_discarded_cond.c
// Pins: 0021: the requires lets y == 1 reach h(y) with c == 0
#include <cerastes.h>

int g(int x)
    CERASTES_REQUIRES(x > 0)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int y)
    CERASTES_REQUIRES(y > 1)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int two_calls(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y > 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? g(x) : h(y);
  return c;
}

int call_or_zero(int c, int x)
    CERASTES_REQUIRES(c == 0 || x > 0)
    CERASTES_ENSURES(r, r == c)
{
  c ? g(x) : 0;
  return c;
}
