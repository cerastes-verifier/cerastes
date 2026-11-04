// expect: error check @ L23
// mode: c23, c++20
// twin: tests/memory/heap_loop_local.c
// Pins: 0047 item 10: the loop writes the block's cell, so its value after the loop is not the one before
#include <cerastes.h>
#include <stdlib.h>

int changed(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
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
    *p = 1;
    i += 1;
  }
  CERASTES_CHECK(*p == 0);
  free(p);
  return 0;
}
