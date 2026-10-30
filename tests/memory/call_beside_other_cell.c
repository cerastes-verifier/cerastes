// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 4: a call that owns *p beside accesses of other cells (x + bump(&y)) and a store to the cell the call changes (x = bump(&x): C orders the store after the call), both verified
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int beside(void)
    CERASTES_ENSURES(r, r == 6)
{
  int x = 1;
  int y = 2;
  int s = x + bump(&y);
  x = bump(&x);
  return s + x;
}
