// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 items 4, 5, 7: ref_to_uninit alone: the callee writes *p and then reads it (the write gives it a value), or leaves it alone; the caller passes an uninitialized local with no flag row
#include <cerastes.h>

int fill(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 7)
{
  *p = 7;
  return *p;
}

void leave_alone(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

int use_fill(void)
    CERASTES_ENSURES(r, r == 7)
{
  int x;
  leave_alone(&x);
  int y = fill(&x);
  return y;
}
