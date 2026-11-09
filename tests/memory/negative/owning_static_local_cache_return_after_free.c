// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/owning_static_local_cache.c
// Pins: 0054 item 5 ("Returns"): the function frees the cache's block and returns with the static local still non-null: it returns while the owning static local does not hold its block
// Pre-registered from record 0054 item 5 (S39 b1y's counterpart for a static local): permission at the return
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
  int a = next();
  int b = next();
  return a + b;
}
