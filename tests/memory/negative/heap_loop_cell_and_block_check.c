// expect: error check @ L27
// mode: c23, c++20
// twin: tests/memory/heap_loop_cell_and_block.c
// Pins: 0047 item 10: the parameter's cell after the loop is not 1 for every n
// Pre-registered from the loops branch's positive (record 0047 item 10), written after it: the expectation was verified once on the single file before it was written
#include <cerastes.h>
#include <stdlib.h>

int mix(int *r, int n)
    CERASTES_REQUIRES(cerastes_owns(*r))
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*r))
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    *r = *p + 1;
    i += 1;
  }
  CERASTES_CHECK(*r == 1);  // <- here
  free(p);
  return 0;
}
