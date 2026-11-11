// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 11 (P78 (c)): sizeof(a) / sizeof(a[0]) of an array local folds to the element count (it keeps its expr); as a loop bound it is 3, so a[i] stays inside the array
// Pre-registered from record 0059 item 11 (Part E), before the code: verifies
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 9)
{
  int a[3] = {1, 2, 3};
  int n = (int)(sizeof(a) / sizeof(a[0]));
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 3 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
