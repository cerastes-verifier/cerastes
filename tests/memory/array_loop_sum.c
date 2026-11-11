// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a read-only loop over int a[3] = {1, 2, 3} with a bound on the sum: no write reaches the array, so its values are kept and each element is at most 3
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 9)
{
  int a[3] = {1, 2, 3};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 3 * i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
