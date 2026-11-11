// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 to 3: int a[] = {...} takes its bound from the list
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int unsized(void)
    CERASTES_ENSURES(r, r == 4)
{
  int a[] = {1, 2, 3, 4};
  return a[3];
}
