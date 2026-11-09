// expect: error invariant-preserved @ L27
// mode: c23, c++20
// twin: tests/memory/owning_static_local_loop_sum.c
// Pins: 0054 item 5 and audit round 2 (Parts C and D), the il-analyst's probe h5: the invariant claims 0 <= s, but next() may return -1 (malloc failed), so it is not preserved
// Pre-registered from record 0054 item 5: invariant-preserved at the invariant clause
#include <cerastes.h>
#include <stdlib.h>

int next(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING static int *cache = NULL;
  if (cache == NULL) {
    cache = (int *)malloc(sizeof(int));
    if (cache == NULL) {
      return -1;
    }
  }
  *cache = 1;
  return *cache - 1;
}

int main(void)
{
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && 0 <= s && s <= 0)  // <- here
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += next();
    i += 1;
  }
  return s;
}
