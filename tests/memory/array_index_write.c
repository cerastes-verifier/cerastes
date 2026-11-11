// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 4: a write to a[i] under 0 <= i < 3, then a read of the same element
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int put(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {0};
  a[i] = 7;
  return a[i];
}
