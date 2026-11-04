// expect: error check @ L27
// mode: c23, c++20
// twin: tests/memory/heap_loop_alias_cell.c
// Pins: 0047 item 10: the cell's value after the loop is the last i, not 0
// Pre-registered from the loops branch's positive (record 0047 item 10), written after it: the expectation was verified once on the single file before it was written
#include <cerastes.h>
#include <stdlib.h>

int alias(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = &x;
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    *q = i;
    i += 1;
  }
  CERASTES_CHECK(x == 0);  // <- here
  free(p);
  return x;
}
