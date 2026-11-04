// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12: a written atom on **pp in an invariant (pp a local pointer to the address-taken cell p): its object is the chained read of *pp through the automatic places
#include <cerastes.h>
#include <stddef.h>

int walk2(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 1;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && pp == &p && *pp == &x && cerastes_owns(**pp) && **pp == 1)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return **pp;
}
