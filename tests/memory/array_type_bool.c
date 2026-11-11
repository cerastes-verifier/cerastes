// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 1 and 2: a bool element (the bool cell type's value and flag fields)
// Pre-registered from record 0059 item 2: verifies
#include <cerastes.h>

int flags(void)
    CERASTES_ENSURES(r, r == 1)
{
  bool a[3] = {true, false, true};
  a[1] = a[0];
  return a[1] ? 1 : 0;
}
