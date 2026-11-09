// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 ("Loops (as built)") and audit round 2 (Parts C and D), the il-analyst's probe h5: the owning static local cache called in a loop that sums its results (each is 0 or -1), with the invariant -i <= s <= 0: the loop must keep the static local's state across the call and the sum bounded
// Pre-registered from record 0054 item 5 and the il-analyst's probe h5 (invariant -i <= s instead of -3 <= s): verifies in both modes
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
  CERASTES_INVARIANT(0 <= i && i <= 3 && -i <= s && s <= 0)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += next();
    i += 1;
  }
  return s;
}
