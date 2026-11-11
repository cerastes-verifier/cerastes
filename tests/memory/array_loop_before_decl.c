// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 6 and 8 (audit round 2, item 5e): a loop before an array's declaration in its own block: the block's array is allocated at the block's entry, so the loop's places hold it (allocated, flags false): the loop does not touch it
// Pre-registered from audit round 2 of record 0059 (item 5e): verifies once Part C's lowering handles the block-entry allocation
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 7)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  while (s < 3) {
    s++;
  }
  int a[3] = {1, 2, 3};
  return a[0] + a[2] + s;
}
