// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 6 and 8 (audit round 2, item 5e): an array declared in a for loop's init-statement: its block is the for's, entered before the loop and left after it, and the loop writes it
// Pre-registered from audit round 2 of record 0059 (item 5e): verifies once the encoder pushes the for's target before the loop (an exit 3 on the audit's build)
#include <cerastes.h>

int f(void)
    CERASTES_ENSURES(r, r == 3)
{
  int s = 0;
  CERASTES_INVARIANT(0 <= s && s <= 3)
  CERASTES_DECREASES(3 - s)
  for (int a[2] = {1, 2}; s < 3; s++) {
    a[0] = s;
    if (a[0] != s) {
      return -1;
    }
  }
  return s;
}
