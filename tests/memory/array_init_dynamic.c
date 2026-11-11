// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 3: non-constant elements are stored in the IL's order, the rest as written
// Pre-registered from record 0059 item 4: verifies
#include <cerastes.h>

int dynamic(int x, int y)
    CERASTES_REQUIRES(0 <= x && x < 100 && 0 <= y && y < 100)
    CERASTES_ENSURES(r, r == x + y + 1)
{
  int a[3] = {x, y + 1, 7};
  return a[0] + a[1];
}
