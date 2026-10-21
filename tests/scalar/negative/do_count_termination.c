// expect: error termination @ L14
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0014 item 5 and 0013: every iteration followed by another must decrease the measure
// The body leaves i alone, so the condition i < n repeats the iteration forever.
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(1 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i < n)
  CERASTES_DECREASES(n - i)
  do {
    i += 0;
  } while (i < n);
  return i;
}
