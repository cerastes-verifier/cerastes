// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_inner.c
// Pins: 0045 item 10: the invariant claims **pp == 0, the body increments it: invariant-preserved at the clause
#include <cerastes.h>
#include <stddef.h>

void bump_n(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp) && 0 <= n && n <= 100 && **pp == 0)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && cerastes_owns(**pp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp == 0)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp += 1;
    i += 1;
  }
}
