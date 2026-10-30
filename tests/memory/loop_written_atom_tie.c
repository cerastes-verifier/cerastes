// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0040 item 5 (S28 P8): the loop assigns p, so the user's invariant says where it ends (p == q, q the entry value); live(q) holds q's object for the copy p = q; the permission comes back for the ensures
#include <cerastes.h>
#include <stddef.h>

int tie(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int *q = p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == q && cerastes_live(q) && cerastes_owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    p = q;
    i += 1;
  }
  return i;
}
