// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a fill loop over an initialized array: the loop writes a[i], so its range place keeps the permission, the flags and the type's range, not the values
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 3)
    CERASTES_ENSURES(r, r == n)
{
  int a[3] = {0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    a[i] = i;
    i = i + 1;
  }
  return i + a[0] * 0;
}
