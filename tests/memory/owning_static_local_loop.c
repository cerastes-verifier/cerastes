// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 and audit round 2 (Parts C and D), probe h10: the owning static local cache of owning_static_local_cache.c, its function called in a loop: the loop must not forget that the static local holds a block or is null
// Pre-registered from record 0054 item 5 (a loop whose call names the static local through the implicit atom): verifies in both modes (audit probe h10; an uninitialized false alarm at the call when written)
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
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    (void)next();
    i += 1;
  }
  return 0;
}
