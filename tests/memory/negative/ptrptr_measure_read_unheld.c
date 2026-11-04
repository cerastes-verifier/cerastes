// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/ptrptr_measure_read.c
// Pins: 0045 item 12: the measure reads **pp, which the loop does not hold: permission at the clause
#include <cerastes.h>
#include <stddef.h>

void count(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && 0 <= n && n <= 100)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - **pp)  // <- here
  while (i < n) {
    i += 1;
  }
}
