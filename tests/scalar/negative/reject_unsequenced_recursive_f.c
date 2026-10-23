// expect: rejected cerastes-unsequenced_calls @ L25
// mode: c23, c++20
// twin: tests/scalar/call_reorder.c
// Pins: 0021 item 4: f(g(x)) + 100 / h(y) with f in the caller's own cycle: the termination check of the recursive call f needs g first, and the division needs h first
#include <cerastes.h>

int g(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int h(int y)
    CERASTES_ENSURES(r, r == y)
{
  return y;
}

int f(int n)
    CERASTES_DECREASES(n)
{
  if (n <= 1) {
    return 0;
  }
  return f(g(n - 1)) + 100 / h(n);
}
