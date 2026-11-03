// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/read_double_deref_invariant.c
// Pins: 0045 item 12: the invariant reads **pp, which the loop does not hold (owns(*pp) holds the pointer, not the object): permission at the clause
#include <cerastes.h>
#include <stddef.h>

void walk(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && 0 <= n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp >= 0)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}
