// expect: error invalid-free @ L26
// mode: c23, c++20
// twin: tests/memory/heap_loop_param_cursor.c
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): the block the cursor parameter was pointed at is freed twice
// Pre-registered from .claude/plans/m4-5d2v-allocator.md Part 1 (t19's twin): invalid-free at the second free only
#include <cerastes.h>
#include <stdlib.h>

int f(int *t, int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
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
    t = p;
    *t = i;
    i += 1;
  }
  free(p);
  free(p);  // <- here
  return 0;
}
