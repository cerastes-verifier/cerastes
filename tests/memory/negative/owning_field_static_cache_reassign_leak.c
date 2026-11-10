// expect: error leak @ L18
// mode: c23, c++20
// twin: tests/memory/owning_field_static_cache.c
// Pins: 0055 item 5 (reassignment of a static leaf): without the "if (cache.data == NULL)" guard the function overwrites the block the cache may hold from an earlier call (owning_static_local_cache_reassign_leak.c)
// Pre-registered from record 0055 item 5 (named leaves: new, no base form): leak at the store to cache.data (the reassignment row over the leaf's entry value, held where the implicit atom gives it)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int next(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  static struct buf cache;
  cache.data = (int *)malloc(sizeof(int));  // <- here
  if (cache.data == NULL) {
    return -1;
  }
  *cache.data = 1;
  return *cache.data - 1;
}

int main(void)
{
  int a = next();
  int b = next();
  return a + b;
}
