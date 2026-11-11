// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8 (audit round 3, item 2): a loop measure that reads an element: CERASTES_DECREASES(a[0]) with a[0]-- in the body: framed by the range place, its flag set, decreasing and non-negative by the invariant
// Pre-registered from audit round 3 of record 0059: verifies
#include <cerastes.h>

int countdown(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[1] = {3};
  CERASTES_INVARIANT(a[0] >= 0)
  CERASTES_DECREASES(a[0])
  while (a[0] > 0) {
    a[0]--;
  }
  return a[0];
}
