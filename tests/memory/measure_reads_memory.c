// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 8 and 0038 item 8: decreases(*p) over an owned cell: the recursive call's measure is read in the callee's entry state; a non-recursive function may also have one (re-expects the retired pin of 0037 items 5 and 15)
#include <cerastes.h>
#include <stddef.h>

int down(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0 && *p < 100)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  if (*p == 0) {
    return 0;
  }
  *p -= 1;
  return down(p);
}

int leaf(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p >= 0)
    CERASTES_DECREASES(*p)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == *p)
{
  return *p;
}
