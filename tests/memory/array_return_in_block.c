// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 6: a return inside the array's block leaves it: the scope exit runs before the return
// Pre-registered from record 0059 item 6: verifies
#include <cerastes.h>

int early(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 1 : 0))
{
  if (c != 0) {
    int a[2] = {1, 2};
    return a[0];
  }
  return 0;
}
