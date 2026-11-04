// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/memory/read_double_deref_invariant.c
// Pins: 0045 item 12 (P69's trap extended): the loop repoints *pp to y and writes y, so **pp == x is not preserved: invariant-preserved
#include <cerastes.h>

void walk(int n) CERASTES_REQUIRES(0 <= n)
{
  int x = 0;
  int y = 0;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp == x)  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pp = &y;
    y = i + 1;
    i += 1;
  }
}
