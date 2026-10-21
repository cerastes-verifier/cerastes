// expect: rejected cerastes-missing_decreases @ L14
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0013 item 3 and 0014 item 5: a do loop with a non-constant condition needs a decreases
// Only do ... while (0) is exempt.
#include <cerastes.h>

int count_to(int n)
    CERASTES_REQUIRES(1 <= n && n <= 1000)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < n)
  do {
    i += 1;
  } while (i < n);
  return i;
}
