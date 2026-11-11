// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8 and 0032 item 8: a return inside a loop over an array: the scope exit of the array runs at the return
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int find(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 3)
{
  int a[3] = {5, 1, 2};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i] == 1) {
      return i;
    }
    i = i + 1;
  }
  return 3;
}
