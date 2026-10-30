// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 4 (audit S2): *first(&x) as a value and as the target of +=: the place is the call's result, so the access follows the call (no unsequenced report); first returns its parameter and keeps the cell
#include <cerastes.h>
#include <stddef.h>

int *first(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, r == p && cerastes_owns(*p) && *p == cerastes_old(*p))
{
  return p;
}

int use_first(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 1;
  int v = *first(&x);
  *first(&x) += 3;
  return x + v;
}
