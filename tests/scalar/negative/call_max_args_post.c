// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/scalar/call_max_args.c
// Pins: 0021: the result of the third call is the maximum of the two results plus nothing (off by one in the contract)
#include <cerastes.h>

int succ(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  return x + 1;
}

int larger(int p, int q)
    CERASTES_ENSURES(r, r == (p > q ? p : q))
{
  return p > q ? p : q;
}

int test(int a, int b)
    CERASTES_REQUIRES(a < 2147483647 && b < 2147483647)
    CERASTES_ENSURES(r, r == (a > b ? a : b))
{
  return larger(succ(a), succ(b));
}
