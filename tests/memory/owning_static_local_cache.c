// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 (retires reject_owning_static_local.c, whose function tests s == NULL): an owning static local as a cache: the implicit atom owns(cache) of the function and of every caller carries the deep atom (g == NULL || owns_block(g)), the function allocates on first use and keeps the block, main calls it twice and ends holding the block (exempt)
// Pre-registered from record 0054 item 5 ("the implicit owns(s) of a static local", question 80): verifies in both modes
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
  int a = next();
  int b = next();
  return a + b;
}
