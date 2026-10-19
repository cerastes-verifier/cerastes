// expect: error termination @ L16
// mode: c23, c++20
// twin: tests/scalar/measure_not_needed.c
// Pins: 0013 item 4: the caller needs no measure, but the recursive callee's measure must decrease
// The error is in the callee, at its recursive call.
#include <cerastes.h>

int countdown_leaf(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return countdown_leaf(n);
}

int plain_caller(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == 0)
{
  return countdown_leaf(n);
}
