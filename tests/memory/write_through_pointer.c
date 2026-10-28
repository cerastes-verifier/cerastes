// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 4, 6: a write through a pointer to an address-taken local is seen by the local
#include <cerastes.h>

int write_through(void)
    CERASTES_ENSURES(r, r == 2)
{
  int x = 1;
  int *p = &x;
  *p = 2;
  CERASTES_CHECK(x == 2);
  return x;
}
