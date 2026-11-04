// expect: error termination @ L15
// mode: c23, c++20
// twin: tests/memory/ptrptr_measure_read.c
// Pins: 0045 item 12: the measure n - **pp never changes (the cell is not written): termination at the clause
#include <cerastes.h>

void count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  int **pp = &p; if (pp) { }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp == 0)
  CERASTES_DECREASES(n - **pp)  // <- here
  while (i < n) {
    i += 1;
  }
}
