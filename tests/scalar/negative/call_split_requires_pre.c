// expect: error precondition @ L31
// mode: c23, c++20
// twin: tests/scalar/call_split_requires.c
// Pins: 0026 item 6: y may be 1, so the clause 2 <= b of take fails at the call
#include <cerastes.h>

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

int take(int a, int b)
    CERASTES_REQUIRES(1 <= a)
    CERASTES_REQUIRES(2 <= b)
    CERASTES_ENSURES(r, r == a - b)
{
  return a - b;
}

int test(int x, int y)
    CERASTES_REQUIRES(1 <= x && 1 <= y)
    CERASTES_ENSURES(r, r == x - y)
{
  return take(f(x), h(y));
}
