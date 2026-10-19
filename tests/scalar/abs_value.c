// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 3 and 0012 item 5: the conditional operator checks its branch only when taken
#include <cerastes.h>

int absolute(int a)
    CERASTES_REQUIRES(a >= -2147483647)
    CERASTES_ENSURES(r, r >= 0)
{
  return a < 0 ? -a : a;
}
