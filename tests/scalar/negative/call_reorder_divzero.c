// expect: error division-by-zero @ L28
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021: y may be 0; the division by h(y) is checked although f(g(x)) is another operand
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
    CERASTES_REQUIRES(0 <= x && x <= 2147483547)
{
  return f(g(x)) + 100 / h(y);
}
