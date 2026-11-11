// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 4 and 10: a loop that takes p = &a[i] and reads *p: the read-only loop keeps the values, p is a local of the body
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code: verifies
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 9)
{
  int a[3] = {1, 2, 3};
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 3 * i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    int *p = &a[i];
    s = s + *p;
    i = i + 1;
  }
  return s;
}
