// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 items 3 and 4: a[i] under 0 <= i < 3 passes the bound row; {0} writes one element and fills the rest with zero
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int at(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {0};
  return a[i];
}
