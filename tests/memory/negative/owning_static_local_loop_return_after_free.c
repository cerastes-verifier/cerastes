// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/owning_static_local_loop.c
// Pins: 0054 item 5 ("Returns") and audit round 2 (Parts C and D), probe h10: the cache function frees its block and returns with the static local still non-null, called from a loop
// Pre-registered from record 0054 item 5: permission at the return of next (the deep atom's return row)
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
  int v = *cache - 1;
  free(cache);
  return v;  // <- here
}

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    (void)next();
    i += 1;
  }
  return 0;
}
