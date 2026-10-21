// expect: error invariant-entry @ L13
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0014 item 5: the invariant must hold before the first iteration
// n == 0 breaks i < n on entry (the body would still run once).
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i < n)
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
  } while (i < n);
  return i;
}
