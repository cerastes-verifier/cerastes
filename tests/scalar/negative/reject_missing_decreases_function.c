// expect: rejected cerastes-missing_decreases @ L8
// mode: c23, c++20
// twin: tests/scalar/recursion.c
// Pins: 0013 item 4: a recursive function without a decreases is rejected
// The error is at the function's definition.
#include <cerastes.h>

int count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return count_down(n - 1);
}
