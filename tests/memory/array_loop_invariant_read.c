// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: an invariant that reads a[i]: framed by the range place, its index inside the array, its flag set (the array is initialized)
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int first(void)
    CERASTES_ENSURES(r, r >= 1)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(a[i] >= 1)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return a[i];
}
