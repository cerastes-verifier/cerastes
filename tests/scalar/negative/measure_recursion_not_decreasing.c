// expect: error termination @ L16
// mode: c23, c++20
// twin: tests/scalar/recursion.c
// Pins: 0013 item 2: the recursive call has the same measure
// The termination error is at the call.
#include <cerastes.h>

int count_down(int n)
    CERASTES_REQUIRES(n >= 0)
    CERASTES_DECREASES(n)
    CERASTES_ENSURES(r, r == 0)
{
  if (n == 0) {
    return 0;
  }
  return count_down(n);
}
