// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4 (as updated): a[i] on the right of i < 3 && is reached only when i < 3: the guard carries the bound row (i may be 3 on entry)
// Pre-registered from record 0059 item 4 (as updated): verifies
#include <cerastes.h>

int guarded(int i)
    CERASTES_REQUIRES(0 <= i && i <= 3)
    CERASTES_ENSURES(r, r == (i < 3))
{
  int a[3] = {0};
  return i < 3 && a[i] == 0;
}
