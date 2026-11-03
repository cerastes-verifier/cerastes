// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/heap_guard_invariant.c
// Pins: 0045 items 11, 12: the invariant's atom needs **pp when *pp is non-null, which the function does not own: permission at the clause (entry)
#include <cerastes.h>
#include <stddef.h>

void fill(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (*pp == NULL || cerastes_owns(**pp)))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
