// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 2 and 3: a pointer comparison in a loop condition, both parameters own their cells (no one-past-the-end): the check holds
#include <cerastes.h>

int walk(int *p, int *q, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(*q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(*q) && r == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n && p != q) {
    i += 1;
  }
  return i;
}
