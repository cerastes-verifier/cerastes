// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 4 (audit B1): the read of x in the right operand of && follows the call (the call is in the left operand, sequenced before it): bump makes x 1, so x == 0 is false; a guarded read of *p beside a call on another cell
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int and_after_call(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  return bump(&x) && x == 0;
}

int guarded_then_call(int *p, int *q)
    CERASTES_REQUIRES((p == NULL || cerastes_owns(*p)) && cerastes_owns(*q) && *q < 100)
    CERASTES_ENSURES(r, (p == NULL || cerastes_owns(*p)) && cerastes_owns(*q))
{
  return (p == NULL || *p > 0) && bump(q);
}
