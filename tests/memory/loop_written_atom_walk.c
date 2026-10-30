// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 (S28 P8): a pointer local walking two cells, the user's tie (q == p || q == &y) in the invariant; the atom frames the read of *q and the return reads *q
#include <cerastes.h>
#include <stddef.h>

int walk(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p == 1 && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 1)
{
  int y = 1;
  int *q = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (q == p || q == &y) && cerastes_owns(*q) && *q == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = (q == p) ? &y : p;
    i += 1;
  }
  return *q;
}
