// expect: error leak @ L13
// mode: c23, c++20
// twin: tests/memory/owning_static_local_cache.c
// Pins: 0054 item 5: without the "if (cache == NULL)" guard the function overwrites a block the cache may hold from an earlier call
// Pre-registered from record 0054 item 5 ("Stores"): leak at the assignment
#include <cerastes.h>
#include <stdlib.h>

int next(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING static int *cache = NULL;
  cache = (int *)malloc(sizeof(int));  // <- here
  if (cache == NULL) {
    return -1;
  }
  *cache = 1;
  return *cache - 1;
}

int main(void)
{
  int a = next();
  int b = next();
  return a + b;
}
