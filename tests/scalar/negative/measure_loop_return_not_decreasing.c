// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/measure_loop_return.c
// Pins: 0013 item 8: below 50 the loop forgets to count, so only the return can end it
// The loop does not terminate for n > 0 (it spins below 50 and returns at 50 only if i got there); the measure does not decrease.
#include <cerastes.h>

int first_big(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r >= 0 && r <= n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i >= 50) {
      return i;
    }
  }
  return i;
}
