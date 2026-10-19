// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0012 item 7: x op= e is x = (T)(x op e) for each operator, one after the other
#include <cerastes.h>

int mix(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 1000)
    CERASTES_ENSURES(r, r == x * 2)
{
  x += 1;
  x *= 2;
  x -= 2;
  return x;
}
