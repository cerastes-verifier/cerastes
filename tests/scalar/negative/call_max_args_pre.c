// expect: error precondition @ L24
// mode: c23, c++20
// twin: tests/scalar/call_max_args.c
// Pins: 0021: the precondition of the second sibling call is checked (b may be 1001)
#include <cerastes.h>

int succ(int x)
    CERASTES_REQUIRES(0 <= x && x <= 1000)
    CERASTES_ENSURES(r, r == x + 1 && 1 <= r && r <= 1001)
{
  return x + 1;
}

int larger(int p, int q)
    CERASTES_ENSURES(r, r == (p > q ? p : q))
{
  return p > q ? p : q;
}

int test(int a, int b)
    CERASTES_REQUIRES(0 <= a && a <= 1000 && 0 <= b && b <= 1001)
    CERASTES_ENSURES(r, r == (a > b ? a : b) + 1)
{
  return larger(succ(a), succ(b));
}
