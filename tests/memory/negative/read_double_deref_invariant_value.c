// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/memory/read_double_deref_invariant.c
// Pins: 0045 item 12: the invariant claims **pp == 0, the body writes i + 1: invariant-preserved at the clause
#include <cerastes.h>

void walk(int n) CERASTES_REQUIRES(0 <= n)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp == 0)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i + 1;
    i += 1;
  }
}
